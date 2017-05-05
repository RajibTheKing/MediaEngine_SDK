
#include "SendingThreadOfCall.h"
#include "Size.h"
//#include "PacketHeader.h"
#include "VideoHeader.h"
#include "CommonElementsBucket.h"
#include "VideoCallSession.h"
#include "Controller.h"

#include <vector>

#include "LiveReceiver.h"
#include "LiveVideoDecodingQueue.h"
#include "Globals.h"
#include "InterfaceOfAudioVideoEngine.h"

extern CInterfaceOfAudioVideoEngine *G_pInterfaceOfAudioVideoEngine;

//#define SEND_VIDEO_TO_SELF 1
//#define __LIVE_STREAMIN_SELF__

//#define __RANDOM_MISSING_PACKET__

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

CSendingThreadOfCall::CSendingThreadOfCall(CCommonElementsBucket* commonElementsBucket, CSendingBuffer *sendingBuffer, CVideoCallSession* pVideoCallSession, bool bIsCheckCall, long long llfriendID, bool bAudioOnlyLive) :
m_pCommonElementsBucket(commonElementsBucket),
m_SendingBuffer(sendingBuffer),
m_bIsCheckCall(bIsCheckCall),
m_nTimeStampOfChunck(-1),
m_nTimeStampOfChunckSend(0),
m_lfriendID(llfriendID)

{
	m_pVideoCallSession = pVideoCallSession;
}

CSendingThreadOfCall::~CSendingThreadOfCall()
{

}

void CSendingThreadOfCall::StopSendingThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThreadOfCall::StopSendingThread() called");

	//if (pInternalThread.get())
	{
		bSendingThreadRunning = false;

		while (!bSendingThreadClosed)
			m_Tools.SOSleep(5);
	}

	//pInternalThread.reset();

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThreadOfCall::StopSendingThread() Sending Thread STOPPPED");
}

void CSendingThreadOfCall::StartSendingThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThreadOfCall::StartSendingThread() called");

	if (pSendingThread.get())
	{
		pSendingThread.reset();

		return;
	}

	bSendingThreadRunning = true;
	bSendingThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t SendingThreadQ = dispatch_queue_create("SendingThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(SendingThreadQ, ^{
		this->SendingThreadProcedure();
	});

#else

	std::thread myThread(CreateVideoSendingThread, this);
	myThread.detach();

#endif

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThreadOfCall::StartSendingThread() Sending Thread started");

	return;
}

void *CSendingThreadOfCall::CreateVideoSendingThread(void* param)
{
	CSendingThreadOfCall *pThis = (CSendingThreadOfCall*)param;
	pThis->SendingThreadProcedure();

	return NULL;
}

#ifdef PACKET_SEND_STATISTICS_ENABLED
long long iPrevFrameNumerOfCall = 0;
int iNumberOfPacketsInLastFrameOfCall = 0;
int iNumberOfPacketsSentFromLastFrameOfCall = 0;
#endif

void CSendingThreadOfCall::SendingThreadProcedure()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThreadOfCall::SendingThreadProcedure() started Sending method");

	Tools toolsObject;
	int packetSize = 0;
	long long lFriendID = m_lfriendID;
	int startFraction = SIZE_OF_INT_MINUS_8;
	int fractionInterval = BYTE_SIZE;
	int frameNumber, packetNumber;
	//CPacketHeader packetHeader;
	CVideoHeader packetHeader;
	std::vector<int> vAudioDataLengthVector;
	int numberOfVideoPackets = 0;
	int frameCounter = 0;
	int packetSizeOfNetwork = m_pCommonElementsBucket->GetPacketSizeOfNetwork();

#ifdef  BANDWIDTH_CONTROLLING_TEST
	m_BandWidthList.push_back(500 * 1024);    m_TimePeriodInterval.push_back(20 * 1000);
	m_BandWidthList.push_back(8 * 1024);    m_TimePeriodInterval.push_back(20 * 1000);
	m_BandWidthList.push_back(3 * 1024);    m_TimePeriodInterval.push_back(100 * 1000);
	/*m_BandWidthList.push_back(5*1024);    m_TimePeriodInterval.push_back(2*1000);
	m_BandWidthList.push_back(500*1024);    m_TimePeriodInterval.push_back(20*1000);
	m_BandWidthList.push_back(5*1024);    m_TimePeriodInterval.push_back(2*1000);
	m_BandWidthList.push_back(500*1024);    m_TimePeriodInterval.push_back(20*1000);
	m_BandWidthList.push_back(5*1024);    m_TimePeriodInterval.push_back(2*1000);
	m_BandWidthList.push_back(500*1024);    m_TimePeriodInterval.push_back(20*1000);
	m_BandWidthList.push_back(5*1024);    m_TimePeriodInterval.push_back(2*1000);
	m_BandWidthList.push_back(500*1024);    m_TimePeriodInterval.push_back(20*1000);
	m_BandWidthList.push_back(5*1024);    m_TimePeriodInterval.push_back(2*1000);
	m_BandWidthList.push_back(500*1024);    m_TimePeriodInterval.push_back(20*1000);
	m_BandWidthList.push_back(5*1024);    m_TimePeriodInterval.push_back(2*1000);
	m_BandWidthList.push_back(500*1024);    m_TimePeriodInterval.push_back(20*1000);
	m_BandWidthList.push_back(5*1024);    m_TimePeriodInterval.push_back(2*1000);*/
	m_BandWidthController.SetTimeInterval(m_BandWidthList, m_TimePeriodInterval);
#endif
    long long llSendingDequePrevTime = 0;

	long long chunkStartTime = m_Tools.CurrentTimestamp();

	while (bSendingThreadRunning)
	{
		//CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThreadOfCall::SendingThreadProcedure() RUNNING Sending method");

		LOGSS("##SS## m_bAudioOnlyLive %d entityType %d callInLiveType %d ", m_bAudioOnlyLive, m_pVideoCallSession->GetEntityType(), m_pVideoCallSession->GetCallInLiveType());

		LOGSS("##SS## m_bPassOnlyAudio %d ", m_bPassOnlyAudio);

		if (m_SendingBuffer->GetQueueSize() == 0)
		{
			CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThreadOfCall::SendingThreadProcedure() NOTHING for Sending method");

			toolsObject.SOSleep(10);
		}
		else 
		{
			chunkStartTime = m_Tools.CurrentTimestamp();
            
            CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThreadOfCall::SendingThreadProcedure() GOT packet for Sending method");		
            
			int timeDiffForQueue = 0;

			packetSize = m_SendingBuffer->DeQueue(lFriendID, m_EncodedFrame, frameNumber, packetNumber, timeDiffForQueue);		
            
			CLogPrinter_WriteLog(CLogPrinter::INFO, QUEUE_TIME_LOG ,"CSendingThreadOfCall::StartSendingThread() m_SendingBuffer " + toolsObject.IntegertoStringConvert(timeDiffForQueue));
            
            //printf("serverType Number %d\n", m_pVideoCallSession->GetServiceType());
            
		{	//packetHeader.setPacketHeader(m_EncodedFrame + 1);

			unsigned char signal = m_pVideoCallSession->GetFPSController()->GetFPSSignalByte();

			{
				m_EncodedFrame[1 + 3] = signal;
			}

#ifdef PACKET_SEND_STATISTICS_ENABLED

			int iNumberOfPackets = -1;

			iNumberOfPackets = packetHeader.getNumberOfPacket();

			pair<long long, int> FramePacketPair = /*toolsObject.GetFramePacketFromHeader(m_EncodedFrame + 1, iNumberOfPackets);*/make_pair(packetHeader.getFrameNumber(), packetHeader.getPacketNumber());

			if (FramePacketPair.first != iPrevFrameNumerOfCall)
			{
				//CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS,"iNumberOfPacketsSentFromLastFrameOfCall = %d, iNumberOfPacketsInLastFrameOfCall = %d, currentframenumber = %d\n",
				//	iNumberOfPacketsSentFromLastFrameOfCall, iNumberOfPacketsInLastFrameOfCall, FramePacketPair.first);

				if (iNumberOfPacketsSentFromLastFrameOfCall != iNumberOfPacketsInLastFrameOfCall)
				{
					CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "CSendingThreadOfCall::StartSendingThread() $$-->******* iNumberOfPacketsSentFromLastFrameOfCall = "
						+ m_Tools.IntegertoStringConvert(iNumberOfPacketsSentFromLastFrameOfCall)
						+ " iNumberOfPacketsInLastFrameOfCall = "
						+ m_Tools.IntegertoStringConvert(iNumberOfPacketsInLastFrameOfCall)
						+ " currentframenumber = "
						+ m_Tools.IntegertoStringConvert(FramePacketPair.first)
						+ " m_SendingBuffersize = "
						+ m_Tools.IntegertoStringConvert(m_SendingBuffer->GetQueueSize()));

				}


				iNumberOfPacketsInLastFrameOfCall = iNumberOfPackets;
				iNumberOfPacketsSentFromLastFrameOfCall = 1;
				iPrevFrameNumerOfCall = FramePacketPair.first;
			}
			else
			{
				iNumberOfPacketsSentFromLastFrameOfCall++;
			}
#endif


			//			CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "CSendingThreadOfCall::StartSendingThread() Parsing..>>>  FN: "+ m_Tools.IntegertoStringConvert(packetHeader.getFrameNumber())
			//														  + "  pNo : "+ m_Tools.IntegertoStringConvert(packetHeader.getPacketNumber())
			//														  + "  Npkt : "+ m_Tools.IntegertoStringConvert(packetHeader.getNumberOfPacket())
			//														  + "  FPS : "+ m_Tools.IntegertoStringConvert(packetHeader.getFPS())
			//														  + "  Rt : "+ m_Tools.IntegertoStringConvert(packetHeader.getRetransSignal())
			//														  + "  Len : "+ m_Tools.IntegertoStringConvert(packetHeader.getPacketLength())
			//														  + " tmDiff : " + m_Tools.IntegertoStringConvert(packetHeader.getTimeStamp()));


#ifdef  BANDWIDTH_CONTROLLING_TEST
			if (m_BandWidthController.IsSendeablePacket(packetSize)) {
#endif

			if (m_bIsCheckCall == LIVE_CALL_MOOD)
			{
//				m_cVH.setPacketHeader(m_EncodedFrame);

//				m_cVH.ShowDetails("Before Sending ");

#if defined(SEND_VIDEO_TO_SELF)

                unsigned char *pEncodedFrame = m_EncodedFrame;
                LOGEF("TheKing--> Processing CALL!!!\n");
				m_pVideoCallSession->PushPacketForMerging(++pEncodedFrame, --packetSize, false);
#else
				//printf("WIND--> SendFunctionPointer with size  = %d\n", packetSize);
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThreadOfCall::SendingThreadProcedure() came for CALL !!!!!!!");

#ifndef NO_CONNECTIVITY
				HITLER("#@#@26022017# SENDING DATA WITH LENGTH = %d", packetSize);
				
				{
					m_pCommonElementsBucket->SendFunctionPointer(m_pVideoCallSession->GetFriendID(), MEDIA_TYPE_VIDEO, m_EncodedFrame, packetSize, 0, std::vector< std::pair<int, int> >());
				}
				
#else
				HITLER("#@#@26022017# SENDING DATA WITH LENGTH = %d", packetSize);
				m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(m_pVideoCallSession->GetFriendID(), packetSize, m_EncodedFrame);
#endif

				//CLogPrinter_WriteLog(CLogPrinter::INFO, PACKET_LOSS_INFO_LOG ," &*&*Sending frameNumber: " + toolsObject.IntegertoStringConvert(frameNumber) + " :: PacketNo: " + toolsObject.IntegertoStringConvert(packetNumber));
#endif
			}

#if 0

                
                
                
                //m_pCommonElementsBucket->SendFunctionPointer(lFriendID, 2, m_EncodedFrame, packetSize);
                
                //unsigned char *pEncodedFrame = m_EncodedFrame;
                //m_pVideoCallSession->PushPacketForMerging(++pEncodedFrame, --packetSize, true);
                
                /*
                if(m_pVideoCallSession->GetResolationCheck() == false)
                {
                    unsigned char *pEncodedFrame = m_EncodedFrame;
                    //m_pVideoCallSession->PushPacketForMerging(++pEncodedFrame, --packetSize);
                    
                    
                    //m_pCommonElementsBucket->SendFunctionPointer(lFriendID, 2, m_EncodedFrame, PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE);
                }
                else
                {
                    m_pCommonElementsBucket->SendFunctionPointer(lFriendID, 2, m_EncodedFrame, packetSize);
                    
                    //CLogPrinter_WriteLog(CLogPrinter::INFO, PACKET_LOSS_INFO_LOG ," &*&*Sending frameNumber: " + toolsObject.IntegertoStringConvert(frameNumber) + " :: PacketNo: " + toolsObject.IntegertoStringConvert(packetNumber));
                }
                */
                
#endif
				toolsObject.SOSleep(0);

#ifdef  BANDWIDTH_CONTROLLING_TEST
			}
#endif
        }


		}
	}

	bSendingThreadClosed = true;

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThreadOfCall::SendingThreadProcedure() stopped SendingThreadProcedure method.");
}

int CSendingThreadOfCall::GetSleepTime()
{
	int SleepTime = MAX_VIDEO_PACKET_SENDING_SLEEP_MS - (m_pVideoCallSession->GetVideoEncoder()->GetBitrate() / REQUIRED_BITRATE_FOR_UNIT_SLEEP);

	if(SleepTime < MIN_VIDEO_PACKET_SENDING_SLEEP_MS)
		SleepTime = MIN_VIDEO_PACKET_SENDING_SLEEP_MS;

	return SleepTime;
}

