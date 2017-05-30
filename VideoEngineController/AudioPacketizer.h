#ifndef _AUDIO_PACKETIZER_H_
#define _AUDIO_PACKETIZER_H_

#include "AudioMacros.h"
#include "SmartPointer.h"
#include "AudioPacketHeader.h"

namespace MediaSDK
{

	class CAudioCallSession;
	class AudioPacketHeader;

	class AudioPacketizer
	{
	public:
		AudioPacketizer();
		~AudioPacketizer();

		/*
		Packatize audio data

		@param uchData: Data to be packatized
		@param packatizeData: Output buffer with packatized data
		@return: Data length of packatized data

		*/
		void Packetize(unsigned char* uchData, const AudioHeaderFields& headerParams, PackatizedDataListener* callback);

	private:

		CAudioCallSession* m_pAudioCallSession;
		SmartPointer<AudioPacketHeader> m_AudioPacketHeader;

		int m_nHeaderLengthWithMediaByte, m_nMaxDataSyzeInEachBlock, m_nHeaderLength;

		unsigned char m_uchAudioBlock[MAX_AUDIO_DECODER_FRAME_SIZE + 10];
	};

} //namespace MediaSDK

#endif