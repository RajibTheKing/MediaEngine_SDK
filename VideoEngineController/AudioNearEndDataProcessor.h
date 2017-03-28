#ifndef AUDIO_NEAREND_DATA_PROCESSOR_H
#define AUDIO_NEAREND_DATA_PROCESSOR_H

#include "Size.h"
#include "LockHandler.h"
#include "Tools.h"
#include "SmartPointer.h"
#include <vector>

class CAudioCallSession;
class AudioPacketizer;
class CCommonElementsBucket;
class CAudioPacketHeader;
class CAudioShortBuffer;
class CAudioCodec;
class CAudioNearEndDataProcessor
{
public:

	CAudioNearEndDataProcessor(long long llFriendID, CAudioCallSession *pAudioCallSession, CCommonElementsBucket* pCommonElementsBucket, CAudioShortBuffer *pAudioEncodingBuffer, bool bIsLiveStreamingRunning);
	~CAudioNearEndDataProcessor();

	static void *CreateAudioEncodingThread(void* param);
	void EncodingThreadProcedure();
	void GetAudioDataToSend(unsigned char * pAudioCombinedDataToSend, int &CombinedLength, std::vector<int> &vCombinedDataLengthVector,
		int &sendingLengthViewer, int &sendingLengthCallee);

private:
	void StartEncodingThread();
	void StopEncodingThread();
	void MuxIfNeeded();
	void DumpEncodingFrame();
	void PrintRelativeTime(int &cnt, long long &llLasstTime, int &countFrame, long long & llRelativeTime, long long & llCapturedTime);
	bool PreProcessAudioBeforeEncoding();
	void EncodeIfNeeded(long long &llCapturedTime, long long &encodingTime, double &avgCountTimeStamp);
	void SetAudioIdentifierAndNextPacketType();
	void AddHeader(int &version, long long &llRelativeTime);
	void BuildAndGetHeaderInArray(int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
		int numPacketRecv, int channel, int version, long long timestamp, unsigned char* header);
	void EnqueueReadyToSendData(Tools toolsObject);
	
	long long m_llFriendID;
	bool m_bIsLiveStreamingRunning;
	bool m_bIsReady;
	int m_nEncodedFrameSize;
	int m_nRawFrameSize;
	int m_MyAudioHeadersize;
	int m_iPrevRecvdSlotID;
	int m_iPacketNumber;
	int m_iReceivedPacketsInPrevSlot;
	int m_iNextPacketType;
	int m_iSlotID;
	LongLong m_llMaxAudioPacketNumber;
	LongLong m_llEncodingTimeStampOffset;

	CAudioCodec *m_pAudioCodec;

	CAudioCallSession *m_pAudioCallSession;
	CCommonElementsBucket *m_pCommonElementsBucket;	
	CAudioShortBuffer *m_pAudioEncodingBuffer;
	CAudioPacketHeader *m_pAudioPacketHeader;

	short m_saAudioRecorderFrame[MAX_AUDIO_FRAME_Length];//Always contains UnMuxed Data
	unsigned char m_ucaEncodedFrame[MAX_AUDIO_FRAME_Length];
	short m_saAudioMUXEDFrame[MAX_AUDIO_FRAME_Length];//Always contains data for VIEWER_NOT_IN_CALL, MUXED data if m_saAudioPrevDecodedFrame is available

	bool m_bAudioEncodingThreadRunning;
	bool m_bAudioEncodingThreadClosed;

	unsigned char m_ucaRawFrameMuxed[MAX_AUDIO_FRAME_Length], m_ucaRawFrameNonMuxed[MAX_AUDIO_FRAME_Length];
	int m_iRawDataSendIndexViewer, m_iRawDataSendIndexCallee;

	short m_saAudioPrevDecodedFrame[MAX_AUDIO_FRAME_Length];
	unsigned char m_ucaRawDataToSendCallee[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];
	unsigned char m_ucaRawDataToSendViewer[MAX_AUDIO_DATA_TO_SEND_SIZE + 10];
	std::vector<int> m_vRawFrameLengthViewer, m_vRawFrameLengthCallee;
	
	//SmartPointer<std::thread> m_pAudioEncodingThread;
	SmartPointer<CLockHandler> m_pAudioEncodingMutex;
};

#endif //AUDIO_NEAREND_DATA_PROCESSOR_H