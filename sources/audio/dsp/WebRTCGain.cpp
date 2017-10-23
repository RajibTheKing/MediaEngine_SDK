#include "WebRTCGain.h"

#include "LogPrinter.h"
#include "Tools.h"

#ifdef USE_AGC

#include "gain_control.h"

#endif

#include "AudioMacros.h"

#ifndef ALOG
#define ALOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO, "ALOG:" + a);
#endif
//#define GAIN_DUMP

namespace MediaSDK
{
#ifdef GAIN_DUMP
	FILE* gainIn = nullptr;
	FILE* gainOut = nullptr;
#endif

	WebRTCGain::WebRTCGain() : m_bGainEnabled(true)
	{
#ifdef GAIN_DUMP
		gainIn = fopen("/sdcard/gain.input.pcm", "wb");
		gainOut = fopen("/sdcard/gain.output.pcm", "wb");
        /*const std::string path = std::string(getenv("HOME")) + "/Documents/";
        std::string a = path + "gain.input.pcm";
        std::string b = path + "gain.output.pcm";
		gainIn = fopen(a.c_str(), "wb");
		gainOut = fopen(b.c_str(), "wb");*/
#endif

		m_iSkipFrames = 20; //don't add gain for first 20 frames.

#ifdef USE_AGC
		LOGT("###GN##55 #gain# WebRTCGain::WebRTCGain()");

		m_iVolume = DEFAULT_GAIN;
		//m_sTempBuf = new short[3000]; //For channel max buffer size required is 2048 otherwise 800
		int agcret = -1;

		AGC_instance = WebRtcAgc_Create();

#endif
	}

	void WebRTCGain::Init(int serviceType)
	{
		m_iServiceType = serviceType;
		if (m_iServiceType == SERVICE_TYPE_CHANNEL)
		{
			m_iSampleSize = 160;
		}
		else
		{
			m_iSampleSize = 80;
		}
#ifdef USE_AGC
		
		if (WebRtcAgc_Init(AGC_instance, WEBRTC_AGC_MIN_LEVEL, WEBRTC_AGC_MAX_LEVEL, MODE_ADAPTIVE_DIGITAL, serviceType == SERVICE_TYPE_CHANNEL ? 48000 : AUDIO_SAMPLE_RATE)) //Channel's audio sample rate is 44100 but webrtc fails on 44100
		{
			LOGT("###GN## WebRtcAgc_Init failed");
		}
		else
		{
			LOGT("###GN## WebRtcAgc_Init successful servicetype:%d", serviceType);
		}

		SetGain(m_iVolume);
#endif
	}


	WebRTCGain::~WebRTCGain()
	{
#ifdef USE_AGC
		LOGT("###GN## #gain# WebRTCGain::~WebRTCGain()");

		WebRtcAgc_Free(AGC_instance);
/*
		if (m_sTempBuf)
		{
			delete m_sTempBuf;
		}*/
#endif
		m_iSkipFrames = 0;
#ifdef GAIN_DUMP
		if(gainIn) fclose(gainIn);
		if(gainOut) fclose(gainOut);
#endif
	}


	bool WebRTCGain::SetGain(int iGain)
	{
#ifdef USE_AGC
		if (iGain <= 0)
		{
			m_bGainEnabled = false;
			return false;
		}
		else if (iGain > MAX_GAIN)
		{
			return false;
		}

		m_bGainEnabled = true;

		/*           ************* Applicable For New AGC Lib *************
		// Sets the target peak |level| (or envelope) of the AGC in dBFs (decibels
		// from digital full-scale). The convention is to use positive values. For
		// instance, passing in a value of 3 corresponds to -3 dBFs, or a target
		// level 3 dB below full-scale. Limited to [0, 31].

		// Sets the maximum |gain| the digital compression stage may apply, in dB. A
		// higher number corresponds to greater compression, while a value of 0 will
		// leave the signal uncompressed. Limited to [0, 90].

		// When enabled, the compression stage will hard limit the signal to the
		// target level. Otherwise, the signal will be compressed but not limited
		// above the target level.
		*/
		m_iVolume = iGain;

		WebRtcAgcConfig gain_config;

		gain_config.targetLevelDbfs = 13 - m_iVolume;      /* m_iVolume's range is 1-10 */ /* so effective dbfs range is 12-3 */    /* possible range: 0 - 31 */
		gain_config.compressionGaindB = m_iServiceType == SERVICE_TYPE_CHANNEL ? 38 : 9;  /*For channel gain level 38 is set from hearing experience*/
																										/* possible range: 0 - 90 */
		gain_config.limiterEnable = m_iServiceType == SERVICE_TYPE_CHANNEL ? false : true;

		if (WebRtcAgc_set_config(AGC_instance, gain_config))
		{
			LOGT("###GN## WebRtcAgc_set_config failed  ");
			return false;
		}
		else
		{
			LOGT("###GN## WebRtcAgc_set_config successful");
		}

		//LOGT("###GN## setgain called with %d", iGain);

#endif

		return true;
	}


	bool WebRTCGain::AddFarEnd(short *sInBuf, int nBufferSize)
	{
#ifdef USE_AGC


		if (!m_bGainEnabled)
		{
			return false;
		}

		if(m_iSkipFrames > 0){
			return true;
		}

		LOGT("###GN## #gain# WebRTCGain::AddFarEnd(), %d", nBufferSize);

		for (int i = 0; i < nBufferSize; i += m_iSampleSize)
		{
			if (0 != WebRtcAgc_AddFarend(AGC_instance, sInBuf + i, m_iSampleSize))
			{
				LOGT("###GN## WebRtcAgc_AddFarend failed");
			}
		}
#endif
		return true;
	}


	bool WebRTCGain::AddGain(short *sInBuf, int nBufferSize, bool bPlayerSide, int nEchoStateFlags)
	{
		//if not in call (normal or live), set nEchoStateFlags to 0
		//TODO: handle addgain parameter's for channel
#ifdef USE_AGC

		if (!m_bGainEnabled)
		{
			return false;
		}

		if(m_iSkipFrames > 0){
			m_iSkipFrames--;
			return true;
		}

#ifdef GAIN_DUMP
		fwrite(sInBuf, 2, nBufferSize, gainIn);
#endif

		//int total = nBufferSize;
		uint8_t saturationWarning;
		int32_t inMicLevel;
		int32_t outMicLevel = 0;
		bool bEchoExists, bSucceeded = true;
		int nNumEchoFlags = nBufferSize / m_iSampleSize;
		//int total = 0, counter = 0; //debugging purpose
		int echoStateMask = 1 << (nNumEchoFlags - 1);
		for (int i = 0; i < nBufferSize; i += m_iSampleSize, echoStateMask >> 1)
		{
			bEchoExists = (nEchoStateFlags & echoStateMask);

			if (!bPlayerSide || (bPlayerSide && !bEchoExists))
			{
				inMicLevel = 0;
				int16_t* in_buf_temp = sInBuf + i;
				//int16_t* out_buf_temp = sInBuf + i;
				if (0 != WebRtcAgc_VirtualMic(AGC_instance, (int16_t* const*)&in_buf_temp, 1, m_iSampleSize, inMicLevel, &outMicLevel))
				{
					LOGT("###GN## WebRtcAgc_VirtualMic failed");
					bSucceeded = false;
				}

				//total += outMicLevel; counter++;
				if (0 != WebRtcAgc_Process(AGC_instance, (const int16_t* const*)&in_buf_temp, 1, m_iSampleSize,
					(int16_t* const*)&in_buf_temp, outMicLevel, &inMicLevel, 1, &saturationWarning))
				{
					LOGT("###GN## WebRtcAgc_Process failed");
					bSucceeded = false;
				}
			}
			//total -= m_iSampleSize;
			//LOGT("##TTGN i:%d bufsize:%d total:%d", i, nBufferSize, total);
		}

#ifdef GAIN_DUMP
		fwrite(sInBuf, 2, nBufferSize, gainOut);
#endif

		//LOGT("###GN## #gain# WebRTCGain::AddGain(), size:%d gainLevel:%d unprocessed:%d samplesize:%d", nBufferSize, m_iVolume, total, m_iSampleSize);
		LOGT("###GN## #gain# WebRTCGain::AddGain(), size:%d gainLevel:%d samplesize:%d", nBufferSize, m_iVolume, m_iSampleSize);

		//LOGT("###GN## addgain done with : %d volumeaverage:%d", bSucceeded, (int)(total/counter));
		return bSucceeded;
#else
		return true;
#endif
	}

} //namespace MediaSDK
