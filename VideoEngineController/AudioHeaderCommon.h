#ifndef AUDIO_HEADER_COMMON_H
#define AUDIO_HEADER_COMMON_H


#include "AudioPacketHeader.h"


namespace MediaSDK
{

	class AudioHeaderCommon : public AudioPacketHeader
	{
	public:

		AudioHeaderCommon();
		AudioHeaderCommon(unsigned int * Information);
		AudioHeaderCommon(unsigned char *Header);

		~AudioHeaderCommon();

		void SetHeaderAllInByteArray(unsigned char* header, int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
			int numPacketRecv, int channel, int version, long long timestamp, int iBlockNumber, int nTotalBlocksInThisFrame, int nBlockOffset, int nFrameLength);
		virtual void SetHeaderAllInByteArray(unsigned char* header, const AudioHeaderFields& params);

		void GetHeaderInfoAll(unsigned char* header, int &nHeaderLength, int &nFrameNumber, int &iBlockNumber, int &nNumberOfBlocks, int &nBlockLength, int &iOffsetOfBlock, int &nFrameLength);

		void CopyHeaderToInformation(unsigned char *Header);
		int GetHeaderInByteArray(unsigned char* data);

		int GetHeaderSize();

		void SetInformation(long long Information, int InfoType);
		long long GetInformation(int InfoType);

		long long GetFieldCapacity(int InfoType);

		bool IsPacketTypeSupported(unsigned int PacketType);
		bool IsPacketTypeSupported();

		void showDetails(char prefix[]);

		bool PutInformationToArray(int InfoType);

	protected:
		int CopyInformationToHeader(unsigned int * Information);

	private:
		void InitHeaderBitMap();

	};

} //namespace MediaSDK

#endif

