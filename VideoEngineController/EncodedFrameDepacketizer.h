#ifndef IPV_VIDEO_PACKET_DEPACKETIZER_H
#define IPV_VIDEO_PACKET_DEPACKETIZER_H

//#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string>
#include <map>
#include <set>
#include <vector>

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "ThreadTools.h"
#include "VideoPacketBuffer.h"
#include "Size.h"
#include "Tools.h"
#include "VideoHeader.h"

namespace MediaSDK
{

	namespace IPV
	{
		class thread;
	}

	class CCommonElementsBucket;
	class CVideoCallSession;

	class CEncodedFrameDepacketizer
	{

	public:

		CEncodedFrameDepacketizer(CCommonElementsBucket* sharedObject, CVideoCallSession *pVideoCallSession);
		~CEncodedFrameDepacketizer();

		void ResetEncodedFrameDepacketizer();

		//int Depacketize(unsigned char *in_data, unsigned int in_size, int PacketType, CPacketHeader &packetHeader);
		int Depacketize(unsigned char *in_data, unsigned int in_size, int PacketType, CVideoHeader &packetHeader);
		void MoveForward(long long frame);
		int CreateNewIndex(long long frame);
		void ClearFrame(int index, long long frame);
		int GetReceivedFrame(unsigned char* data, long long &nFramNumber, long long &nEcodingTime, long long nExpectedTime, int nRight, int &nOrientation);

		map<long long, long long> m_mFrameTimeStamp;
		map<long long, int> m_mFrameOrientation;

		Tools m_Tools;

	private:
		int ProcessFrame(unsigned char *data, int index, long long frameNumber, long long &nFramNumber);
		long long GetEncodingTime(long long nFrameNumber);
		int GetOrientation(long long nFrameNumber);
		bool m_bIsDpkgBufferFilledUp;
		long long m_llFirstFrameReceived;

		int SafeFinder(long long Data/*used for Frame*/);

		long long m_iMaxFrameNumRecvd;
		long long m_FirstFrameEncodingTime;

		std::map<long long/*Frame*/, int/*index*/> m_FrameTracker;
		long long m_FrontFrame;
		long long m_BackFrame;
		int m_Counter;
		int m_BufferSize;
		std::set<int> m_AvailableIndexes;

		CCommonElementsBucket* m_pCommonElementsBucket;
		CVideoCallSession* m_VideoCallSession;

		CVideoPacketBuffer m_CVideoPacketBuffer[DEPACKETIZATION_BUFFER_SIZE + 1];

		//CVideoHeader PacketHeader;	// use to hoy na

		unsigned char * m_pPacketToResend;

	protected:

		SmartPointer<CLockHandler> m_pEncodedFrameDepacketizerMutex;

	};

} //namespace MediaSDK

#endif
