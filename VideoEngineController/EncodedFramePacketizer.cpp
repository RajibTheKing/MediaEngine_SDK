#include "EncodedFramePacketizer.h"
#include "CommonElementsBucket.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "VideoPacketBuffer.h"
#include "ResendingBuffer.h"
#include "Globals.h"
#include "BandwidthController.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

extern bool g_bIsVersionDetectableOpponent;
extern unsigned char g_uchSendPacketVersion;
extern int g_uchOpponentVersion;
extern CFPSController g_FPSController;

CEncodedFramePacketizer::CEncodedFramePacketizer(CCommonElementsBucket* sharedObject, CSendingBuffer* pSendingBuffer) :
m_PacketSize(MAX_PACKET_SIZE_WITHOUT_HEADER),
m_pCommonElementsBucket(sharedObject)
{
	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::CEncodedFramePacketizer");

	m_SendingBuffer = pSendingBuffer;

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::CEncodedFramePacketizer Created");
}

CEncodedFramePacketizer::~CEncodedFramePacketizer()
{
	SHARED_PTR_DELETE(m_pEncodedFrameParsingMutex);
}

int CEncodedFramePacketizer::Packetize(LongLong lFriendID, unsigned char *in_data, unsigned int in_size, int frameNumber,
	unsigned int iTimeStampDiff)
{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CEncodedFramePacketizer::Packetize parsing started");

	unsigned char uchOpponentVersion = g_uchSendPacketVersion;

	int nHeaderLenWithoutMedia = PACKET_HEADER_LENGTH_NO_VERSION;

	if (uchOpponentVersion)
		nHeaderLenWithoutMedia = PACKET_HEADER_LENGTH;

	int nPacketHeaderLenghtWithMedia = nHeaderLenWithoutMedia + 1;


	m_PacketSize = MAX_VIDEO_PACKET_SIZE - nPacketHeaderLenghtWithMedia;
	int packetizedSize = m_PacketSize;

	int readPacketLength = 0;

	int numberOfPackets = (in_size + m_PacketSize - 1) / m_PacketSize;

	if (numberOfPackets > MAX_NUMBER_OF_PACKETS)
		return -1;

	CLogPrinter_Write(CLogPrinter::INFO, "CEncodedFramePacketizer::Packetize in_size " + m_Tools.IntegertoStringConvert(in_size) + " m_PacketSize " + m_Tools.IntegertoStringConvert(m_PacketSize));

	for (int packetNumber = 0; readPacketLength < in_size; packetNumber++, readPacketLength += m_PacketSize)
	{
		if (m_PacketSize + readPacketLength > in_size)
			m_PacketSize = in_size - readPacketLength;

		//m_PacketHeader.setPacketHeader(uchOpponentVersion, frameNumber, numberOfPackets, packetNumber, iTimeStampDiff, 0, 0, m_PacketSize + nPacketHeaderLenghtWithMedia);

		m_PacketHeader.setPacketHeader(uchOpponentVersion, frameNumber, numberOfPackets, packetNumber, iTimeStampDiff, 0, 0, uchOpponentVersion ? m_PacketSize + nPacketHeaderLenghtWithMedia : m_PacketSize);


		m_PacketHeader.GetHeaderInByteArray(m_Packet + 1);

		m_Packet[0] = VIDEO_PACKET_MEDIA_TYPE;
		memcpy(m_Packet + nPacketHeaderLenghtWithMedia, in_data + readPacketLength, m_PacketSize);

		//m_pCommonElementsBucket->m_pEventNotifier->firePacketEvent(m_pCommonElementsBucket->m_pEventNotifier->ENCODED_PACKET, frameNumber, numberOfPackets, packetNumber, m_PacketSize, nPacketHeaderLenghtWithMedia + m_PacketSize, m_Packet);

		//		m_PacketHeader.setPacketHeader(m_Packet+1);
		//		CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "$$--> Lenght "+m_Tools.IntegertoStringConvert(m_PacketHeader.getPacketLength())+"  # TS: "+ m_Tools.IntegertoStringConvert(m_PacketHeader.getTimeStamp()));

//		CLogPrinter_WriteInstentTestLog(CLogPrinter::INFO, "CEncodedFramePacketizer::Packetize Queue lFriendID " + Tools::IntegertoStringConvert(lFriendID) + " packetSize " + Tools::IntegertoStringConvert(nPacketHeaderLenghtWithMedia + m_PacketSize));
		m_SendingBuffer->Queue(lFriendID, m_Packet, nPacketHeaderLenghtWithMedia + m_PacketSize, frameNumber, packetNumber);

	}

	return 1;
}












