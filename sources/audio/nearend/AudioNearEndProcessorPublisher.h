#ifndef AUDIO_NEAR_END_PROCEDURE_PUBLISHER_H
#define AUDIO_NEAR_END_PROCEDURE_PUBLISHER_H


#include "AudioNearEndDataProcessor.h"


namespace MediaSDK
{
	class CAudioCallSession;
	class CAudioShortBuffer;
	class AudioMixer;
	class AudioPacketHeader;
	class AudioDeviceInformation;


	class AudioNearEndProcessorPublisher : public AudioNearEndDataProcessor
	{

	public:

		AudioNearEndProcessorPublisher(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SharedPointer<CAudioShortBuffer> pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
		~AudioNearEndProcessorPublisher() { }

		void ProcessNearEndData();

		
	protected:

		bool MuxIfNeeded(short* shPublisherData, short *shMuxedData, int &nDataSizeInByte, int nPacketNumber);


	private:

		short m_saAudioPrevDecodedFrame[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];
		unsigned char m_uchFarEndFrame[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];
		short m_saSendingDataPublisher[AUDIO_MAX_FRAME_LENGTH_IN_BYTE];  //Always contains data for VIEWER_NOT_IN_CALL, MUXED data if m_saAudioPrevDecodedFrame is available

		SharedPointer<AudioMixer> m_pAudioMixer;
		SharedPointer<AudioPacketHeader>m_pHeader;

		AudioDeviceInformation *m_pAudioDeviceInformation;
		
		int m_nTotalSentFrameSize;
	};

}   //namespace MediaSDK

#endif  // !AUDIO_NEAR_END_PROCEDURE_PUBLISHER_H
