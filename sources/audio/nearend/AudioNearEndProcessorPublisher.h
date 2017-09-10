#ifndef AUDIO_NEAR_END_PROCEDURE_PUBLISHER_H
#define AUDIO_NEAR_END_PROCEDURE_PUBLISHER_H


#include "AudioNearEndDataProcessor.h"


namespace MediaSDK
{
	class CAudioCallSession;
	class CAudioShortBuffer;
	class AudioMixer;


	class AudioNearEndProcessorPublisher : public AudioNearEndDataProcessor
	{

	public:

		AudioNearEndProcessorPublisher(int nServiceType, int nEntityType, CAudioCallSession *pAudioCallSession, SharedPointer<CAudioShortBuffer> pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
		~AudioNearEndProcessorPublisher() { }

		void ProcessNearEndData();


	protected:

		bool MuxIfNeeded(short* shPublisherData, short *shMuxedData, int &nDataSizeInByte, int nPacketNumber);


	private:

		short m_saAudioPrevDecodedFrame[MAX_AUDIO_FRAME_Length];
		short m_saSendingDataPublisher[MAX_AUDIO_FRAME_Length];  //Always contains data for VIEWER_NOT_IN_CALL, MUXED data if m_saAudioPrevDecodedFrame is available

		SharedPointer<AudioMixer> m_pAudioMixer;
	};

}   //namespace MediaSDK

#endif  // !AUDIO_NEAR_END_PROCEDURE_PUBLISHER_H
