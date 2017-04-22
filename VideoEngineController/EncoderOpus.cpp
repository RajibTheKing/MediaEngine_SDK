#include "EncoderOpus.h"
#include "AudioCallSession.h"
#include "CommonElementsBucket.h"
#include "DefinedDataTypes.h"
#include "LogPrinter.h"


EncoderOpus::EncoderOpus(CCommonElementsBucket* sharedObject, CAudioCallSession * AudioCallSession, LongLong llfriendID) :
m_pCommonElementsBucket(sharedObject),
m_bAudioQualityLowNotified(false),
m_bAudioQualityHighNotified(false),
m_bAudioShouldStopNotified(false),
m_FriendID(llfriendID)
{
	m_pMediaSocketMutex.reset(new CLockHandler);
	m_pAudioCallSession = AudioCallSession;
	m_inoLossSlot = 0;
	m_ihugeLossSlot = 0;
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCodec::CAudioCodec");
}


EncoderOpus::~EncoderOpus()
{
	opus_encoder_destroy(encoder);

	SHARED_PTR_DELETE(m_pMediaSocketMutex);
}


int EncoderOpus::CreateAudioEncoder()
{
	CLogPrinter_Write(CLogPrinter::INFO, "CAudioCodec::CreateAudioEncoder");

	int error = 0;
	int sampling_rate = AUDIO_SAMPLE_RATE;
	int dummyDataSize = MAX_AUDIO_FRAME_SAMPLE_SIZE;

	for (int i = 0; i < dummyDataSize; i++)
	{
		m_DummyData[i] = rand();
	}

	encoder = opus_encoder_create(AUDIO_SAMPLE_RATE, AUDIO_CHANNELS, AUDIO_APPLICATION, &err);
	if (err<0)
	{
		return EXIT_FAILURE;
	}

	SetBitrate(AUDIO_BITRATE_INIT);
	
	m_iComplexity = 10;
	long long encodingTime = 0;
	while (m_iComplexity >= 2)
	{
		opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(m_iComplexity));
		encodingTime = m_Tools.CurrentTimestamp();
		EncodeAudio(m_DummyData, dummyDataSize, m_DummyDataOut);
		encodingTime = m_Tools.CurrentTimestamp() - encodingTime;
		if (encodingTime > AUDIO_MAX_TOLERABLE_ENCODING_TIME)
		{
			m_iComplexity--;
		}
		else
		{
			break;
		}
	}
}


int EncoderOpus::EncodeAudio(short *in_data, unsigned int in_size, unsigned char *out_buffer)
{
	int nbBytes;
	int nEncodedSize = 0, iFrameCounter = 0, nProcessedDataSize = 0;

	while (nProcessedDataSize + AUDIO_FRAME_SIZE <= in_size)
	{
		nbBytes = opus_encode(encoder, in_data + iFrameCounter * AUDIO_FRAME_SIZE, AUDIO_FRAME_SIZE, out_buffer + nEncodedSize + 2 * iFrameCounter + 2, AUDIO_MAX_PACKET_SIZE);
		nbBytes = max(nbBytes, 0); //If opus return -1. Not sure about that.
		if (nbBytes == 0)
		{
			//			ALOG( "#EXP#**************************** Encode Failed");
		}
		out_buffer[nEncodedSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter] = (nbBytes & 0x000000FF);
		out_buffer[nEncodedSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter + 1] = (nbBytes >> 8);
		nEncodedSize += nbBytes;
		++iFrameCounter;
		nProcessedDataSize += AUDIO_FRAME_SIZE;
	}

	int nEncodedPacketLength = nEncodedSize + BYTES_TO_STORE_AUDIO_EFRAME_LEN * iFrameCounter;

	if (nEncodedSize < 0)
	{
		fprintf(stderr, "encode failed: %s\n", opus_strerror(nbBytes));
		return EXIT_FAILURE;
	}

	if (nProcessedDataSize != in_size)
	{
		//		ALOG( "#EXP# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^Unused Data");
	}

	return nEncodedPacketLength;
}


bool EncoderOpus::SetBitrate(int nBitrate)
{
	PRT("@@@@@@@@@@@@@@@@@@@@Bitrate: %d\n", nBitrate);
	int ret = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(nBitrate));
	m_iCurrentBitRate = nBitrate;

	return ret != 0;
}


bool EncoderOpus::SetComplexity(int nComplexity)
{
	int ret = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(nComplexity));
	m_iComplexity = nComplexity;

	return ret != 0;
}


int EncoderOpus::GetCurrentBitrate()
{
	return m_iCurrentBitRate;
}


void EncoderOpus::DecideToChangeBitrate(int iNumPacketRecvd)
{
#ifndef AUDIO_FIXED_BITRATE
	//	ALOG("#BR# DecideToChangeBitrate: "+m_Tools.IntegertoStringConvert(iNumPacketRecvd));
	if (iNumPacketRecvd == AUDIO_SLOT_SIZE)
	{
		m_inoLossSlot++;
		m_ihugeLossSlot = 0;
	}
	else
	{
		m_inoLossSlot = 0;
		int nChangedBitRate = (iNumPacketRecvd * m_iCurrentBitRate) / AUDIO_SLOT_SIZE;
		//		ALOG("now br trying to set : "+Tools::IntegertoStringConvert(nChangedBitRate));
		HITLER("@@@@------------------------>Bitrate: %d\n", nChangedBitRate);
		if (nChangedBitRate < AUDIO_LOW_BITRATE && nChangedBitRate >= AUDIO_MIN_BITRATE)
		{
			m_ihugeLossSlot = 0;

			SetBitrate(nChangedBitRate);

			if (false == m_bAudioQualityLowNotified)
			{
				m_pCommonElementsBucket->m_pEventNotifier->fireNetworkStrengthNotificationEvent(m_FriendID, CEventNotifier::NETWORK_STRENTH_GOOD);

				m_bAudioQualityLowNotified = true;
				m_bAudioQualityHighNotified = false;
				m_bAudioShouldStopNotified = false;
			}
		}
		else if (nChangedBitRate < AUDIO_MIN_BITRATE)
		{
			m_ihugeLossSlot++;

			SetBitrate(AUDIO_MIN_BITRATE);

			if (false == m_bAudioShouldStopNotified && m_ihugeLossSlot >= AUDIO_MAX_HUGE_LOSS_SLOT)
			{
				m_pCommonElementsBucket->m_pEventNotifier->fireNetworkStrengthNotificationEvent(m_FriendID, CEventNotifier::NETWORK_STRENTH_BAD);
				m_pCommonElementsBucket->m_pEventNotifier->fireAudioAlarm(AUDIO_EVENT_I_TOLD_TO_STOP_VIDEO, 0, 0);
				m_pAudioCallSession->m_iNextPacketType = AUDIO_NOVIDEO_PACKET_TYPE;

				m_bAudioShouldStopNotified = true;
				m_bAudioQualityHighNotified = false;
				m_bAudioQualityLowNotified = false;
			}
		}
		else if (nChangedBitRate >= AUDIO_LOW_BITRATE)
		{
			m_ihugeLossSlot = 0;

			SetBitrate(nChangedBitRate);

			if (false == m_bAudioQualityHighNotified)
			{
				m_pCommonElementsBucket->m_pEventNotifier->fireNetworkStrengthNotificationEvent(m_FriendID, CEventNotifier::NETWORK_STRENTH_EXCELLENT);

				m_bAudioQualityHighNotified = true;
				m_bAudioQualityLowNotified = false;
				m_bAudioShouldStopNotified = false;
			}
		}
	}

	if (m_inoLossSlot == AUDIO_MAX_NO_LOSS_SLOT)
	{
		if (m_iCurrentBitRate + AUDIO_BITRATE_UP_STEP <= AUDIO_MAX_BITRATE)
		{
			SetBitrate(m_iCurrentBitRate + AUDIO_BITRATE_UP_STEP);
		}
		else
		{
			SetBitrate(AUDIO_MAX_BITRATE);
		}
		m_inoLossSlot = 0;
	}
	//	ALOG("#V# E: DecideToChangeBitrate: Done");
#endif
}


void EncoderOpus::DecideToChangeComplexity(int iEncodingTime)
{
	if (iEncodingTime > AUDIO_MAX_TOLERABLE_ENCODING_TIME && m_iComplexity > OPUS_MIN_COMPLEXITY)
	{
		SetComplexity(m_iComplexity - 1);
	}
	if (iEncodingTime < AUDIO_MAX_TOLERABLE_ENCODING_TIME / 2 && m_iComplexity < OPUS_MAX_COMPLEXITY)
	{
		SetComplexity(m_iComplexity + 1);
	}
}






