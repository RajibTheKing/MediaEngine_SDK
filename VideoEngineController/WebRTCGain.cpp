#include "WebRTCGain.h"

#include "LogPrinter.h"
#include "Tools.h"

#ifdef USE_AGC

#ifdef DESKTOP_C_SHARP
#define OLD_GAIN_LIB
#include "gain_control_old.h"
#else
#include "gain_control.h"
#endif

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
		//gainIn = fopen("/sdcard/gain.input.pcm", "wb");
		//gainOut = fopen("/sdcard/gain.output.pcm", "wb");
        const std::string path = std::string(getenv("HOME")) + "/Documents/";
        std::string a = path + "gain.input.pcm";
        std::string b = path + "gain.output.pcm";
		gainIn = fopen(a.c_str(), "wb");
		gainOut = fopen(b.c_str(), "wb");
#endif

		m_iSkipFrames = 20; //don't add gain for first 20 frames.

#ifdef USE_AGC
		LOGT("###GN##55 #gain# WebRTCGain::WebRTCGain()");

		m_iVolume = DEFAULT_GAIN;
		m_sTempBuf = new short[MAX_AUDIO_FRAME_SAMPLE_SIZE];
		int agcret = -1;

#ifdef OLD_GAIN_LIB
		if ((agcret = WebRtcAgc_Create(&AGC_instance)))
		{
			LOGT("WebRtcAgc_Create failed with error code = %d" , Tools::IntegertoStringConvert(agcret));
		}
		else
		{
			LOGT("WebRtcAgc_Create successful");
		}
#else
		AGC_instance = WebRtcAgc_Create();
#endif

		if ((agcret = WebRtcAgc_Init(AGC_instance, WEBRTC_AGC_MIN_LEVEL, WEBRTC_AGC_MAX_LEVEL, MODE_ADAPTIVE_DIGITAL, AUDIO_SAMPLE_RATE)))
		{
			LOGT("###GN## WebRtcAgc_Init failed");
		}
		else
		{
			LOGT("###GN## WebRtcAgc_Init successful");
		}

		SetGain(m_iVolume);
#endif
	}


	WebRTCGain::~WebRTCGain()
	{
#ifdef USE_AGC
		LOGT("###GN## #gain# WebRTCGain::~WebRTCGain()");

		WebRtcAgc_Free(AGC_instance);
		if (m_sTempBuf)
		{
			delete m_sTempBuf;
		}
#endif
		m_iSkipFrames = 0;
#ifdef GAIN_DUMP
		fclose(gainIn);
		fclose(gainOut);
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
#ifdef OLD_GAIN_LIB
		WebRtcAgc_config_t gain_config;
		m_iVolume = 13;
#else
		WebRtcAgcConfig gain_config;
		m_iVolume = iGain;
#endif

		gain_config.targetLevelDbfs = 13 - m_iVolume;      /* m_iVolume's range is 1-10 */ /* so effective dbfs range is 12-3 */    /* possible range: 0 - 31 */
		gain_config.compressionGaindB = 9;   /* possible range: 0 - 90 */
		gain_config.limiterEnable = true;

		if (WebRtcAgc_set_config(AGC_instance, gain_config))
		{
			LOGT("###GN## WebRtcAgc_set_config failed  ");
			return false;
		}
		else
		{
			LOGT("###GN## WebRtcAgc_set_config successful");
		}

		LOGT("###GN## setgain called with %d", iGain);

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

		for (int i = 0; i < nBufferSize; i += AGC_SAMPLES_IN_FRAME)
		{
			if (0 != WebRtcAgc_AddFarend(AGC_instance, sInBuf + i, AGC_SAMPLES_IN_FRAME))
			{
				LOGT("###GN## WebRtcAgc_AddFarend failed");
			}
		}
#endif
		return true;
	}


	bool WebRTCGain::AddGain(short *sInBuf, int nBufferSize, bool bPlayerSide, int nEchoStateFlags)
	{
#ifdef USE_AGC

		if (!m_bGainEnabled)
		{
			return false;
		}

		if(m_iSkipFrames > 0){
			m_iSkipFrames--;
			return true;
		}

		uint8_t saturationWarning;
		int32_t inMicLevel;
		int32_t outMicLevel = 0;
		bool bSucceeded = true;
		int nNumEchoFlags = nBufferSize / AGC_ANALYSIS_SAMPLES_IN_FRAME;
		//int total = 0, counter = 0; //debugging purpose
		for (int i = 0; i < nBufferSize; i += AGC_ANALYSIS_SAMPLES_IN_FRAME)
		{
			bool bEchoExists = (nEchoStateFlags >> (nNumEchoFlags - i - 1)) & 1;

			if (!bPlayerSide || (bPlayerSide && !bEchoExists))
			{
				inMicLevel = 0;
#ifdef OLD_GAIN_LIB
				if (0 != WebRtcAgc_VirtualMic(AGC_instance, sInBuf + i, 0, AGC_SAMPLES_IN_FRAME, inMicLevel, &outMicLevel))
#else
				int16_t* in_buf_temp = sInBuf + i;
				int16_t* out_buf_temp = m_sTempBuf + i;
				if (0 != WebRtcAgc_VirtualMic(AGC_instance, (int16_t* const*)&in_buf_temp, 1, AGC_SAMPLES_IN_FRAME, inMicLevel, &outMicLevel))
#endif
				{
					LOGT("###GN## WebRtcAgc_VirtualMic failed");
					bSucceeded = false;
				}

				//total += outMicLevel; counter++;
#ifdef OLD_GAIN_LIB
				if (0 != WebRtcAgc_Process(AGC_instance, sInBuf + i, 0, AGC_SAMPLES_IN_FRAME, m_sTempBuf + i, 0,
					outMicLevel, &inMicLevel, 0, &saturationWarning))
#else
				if (0 != WebRtcAgc_Process(AGC_instance, (const int16_t* const*)&in_buf_temp, 1, AGC_SAMPLES_IN_FRAME,
					(int16_t* const*)&out_buf_temp, outMicLevel, &inMicLevel, 1, &saturationWarning))
#endif
				{
					LOGT("###GN## WebRtcAgc_Process failed");
					bSucceeded = false;
				}
			}
		}

#ifdef GAIN_DUMP
		fwrite(sInBuf, 2, nBufferSize, gainIn);
#endif

		memcpy(sInBuf, m_sTempBuf, nBufferSize * sizeof(short));

#ifdef GAIN_DUMP
		fwrite(sInBuf, 2, nBufferSize, gainOut);
#endif

		LOGT("###GN## #gain# WebRTCGain::AddGain(), size:%d gainLevel:%d", nBufferSize, m_iVolume);

		//LOGT("###GN## addgain done with : %d volumeaverage:%d", bSucceeded, (int)(total/counter));
		return bSucceeded;
#else
		return true;
#endif
	}

} //namespace MediaSDK
