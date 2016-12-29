
#include "SendingThread.h"
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

#define NO_CONNECTIVITY

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

CSendingThread::CSendingThread(CCommonElementsBucket* commonElementsBucket, CSendingBuffer *sendingBuffer, CVideoCallSession* pVideoCallSession, bool bIsCheckCall, LongLong llfriendID) :
m_pCommonElementsBucket(commonElementsBucket),
m_SendingBuffer(sendingBuffer),
m_bIsCheckCall(bIsCheckCall),
m_iAudioDataToSendIndex(0),
m_nTimeStampOfChunck(-1),
m_nTimeStampOfChunckSend(0),
m_lfriendID(llfriendID),
m_bInterruptHappened(false),
m_bInterruptRunning(false)

{
	m_pVideoCallSession = pVideoCallSession;
    
    if(pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM)
    {
        llPrevTime = -1;
        m_iDataToSendIndex = 0;
        firstFrame = true;
        m_llPrevTimeWhileSendingToLive = 0;
    }
}

CSendingThread::~CSendingThread()
{

}

void CSendingThread::StopSendingThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThread::StopSendingThread() called");

	//if (pInternalThread.get())
	{
		bSendingThreadRunning = false;

		while (!bSendingThreadClosed)
			m_Tools.SOSleep(5);
	}

	//pInternalThread.reset();

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThread::StopSendingThread() Sending Thread STOPPPED");
}

void CSendingThread::StartSendingThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::StartSendingThread() called");

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

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::StartSendingThread() Sending Thread started");

	return;
}

void *CSendingThread::CreateVideoSendingThread(void* param)
{
	CSendingThread *pThis = (CSendingThread*)param;
	pThis->SendingThreadProcedure();

	return NULL;
}

#ifdef PACKET_SEND_STATISTICS_ENABLED
long long iPrevFrameNumer = 0;
int iNumberOfPacketsInLastFrame = 0;
int iNumberOfPacketsActuallySentFromLastFrame = 0;
#endif

void CSendingThread::SendingThreadProcedure()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::SendingThreadProcedure() started Sending method");

	Tools toolsObject;
	int packetSize;
	LongLong lFriendID;
	int startFraction = SIZE_OF_INT_MINUS_8;
	int fractionInterval = BYTE_SIZE;
	int fpsSignal, frameNumber, packetNumber;
	//CPacketHeader packetHeader;
	CVideoHeader packetHeader;
	std::vector<int> vAudioDataLengthVector;
	int videoPacketSizes[30];
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
	while (bSendingThreadRunning)
	{
		//CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::SendingThreadProcedure() RUNNING Sending method");

		

		if (m_SendingBuffer->GetQueueSize() == 0)
		{
			CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::SendingThreadProcedure() NOTHING for Sending method");

			toolsObject.SOSleep(10);
		}
		else
		{
           
            
            
            CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::SendingThreadProcedure() GOT packet for Sending method");
			
            
			int timeDiffForQueue;
			packetSize = m_SendingBuffer->DeQueue(lFriendID, m_EncodedFrame, frameNumber, packetNumber, timeDiffForQueue);
            
			CLogPrinter_WriteLog(CLogPrinter::INFO, QUEUE_TIME_LOG ,"CSendingThread::StartSendingThread() m_SendingBuffer " + toolsObject.IntegertoStringConvert(timeDiffForQueue));
            
            //printf("serverType Number %d\n", m_pVideoCallSession->GetServiceType());
            
			if ((m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM) && m_pVideoCallSession->GetEntityType() != ENTITY_TYPE_VIEWER_CALLEE )
            {

			LOGEF("fahad -->> m_pCommonElementsBucket 1 --> lFriendID = %lld", lFriendID);
			
			int iIntervalIFrame = m_pVideoCallSession->m_nCallFPS / IFRAME_INTERVAL;
                
            CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::SendingThreadProcedure() session got");

			/*unsigned char *p = m_EncodedFrame + PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE;

			int nalType = p[2] == 1 ? (p[3] & 0x1f) : (p[4] & 0x1f);
			if(nalType == 7) LOGEF("nalType = %d, frameNumber=%d", nalType, frameNumber);*/

			if (frameNumber%iIntervalIFrame == 0 && firstFrame == false)
			{
                CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::SendingThreadProcedure() 200 ms completed");
                
				CAudioCallSession *pAudioSession;

				bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);

				//LOGEF("fahad -->> m_pCommonElementsBucket 2 --> lFriendID = %lld, bExist = %d", lFriendID, bExist);

				if (bExist)
					pAudioSession->getAudioSendToData(m_AudioDataToSend, m_iAudioDataToSendIndex, vAudioDataLengthVector);

				//m_pCommonElementsBucket->SendFunctionPointer(m_VideoDataToSend, m_iDataToSendIndex);
				//m_pCommonElementsBucket->SendFunctionPointer(m_AudioDataToSend, m_iAudioDataToSendIndex);

				long long llNowLiveSendingTimeStamp = m_Tools.CurrentTimestamp();
				long long llNowTimeDiff;

				if (m_llPrevTimeWhileSendingToLive == 0)
				{
					llNowTimeDiff = 0;
					m_llPrevTimeWhileSendingToLive = llNowLiveSendingTimeStamp;
				}
				else
				{
					llNowTimeDiff = llNowLiveSendingTimeStamp - m_llPrevTimeWhileSendingToLive;
					m_llPrevTimeWhileSendingToLive = llNowLiveSendingTimeStamp;
				}

				m_nTimeStampOfChunckSend += llNowTimeDiff;

			//	m_Tools.IntToUnsignedCharConversion(m_iDataToSendIndex, m_AudioVideoDataToSend, 0);
			//	m_Tools.IntToUnsignedCharConversion(m_iAudioDataToSendIndex, m_AudioVideoDataToSend, 4);

				m_Tools.SetMediaUnitVersionInMediaChunck(0, m_AudioVideoDataToSend);
				m_Tools.SetMediaUnitTimestampInMediaChunck(m_nTimeStampOfChunck, m_AudioVideoDataToSend);
				m_Tools.SetAudioBlockSizeInMediaChunck(m_iAudioDataToSendIndex, m_AudioVideoDataToSend);
				m_Tools.SetVideoBlockSizeInMediaChunck(m_iDataToSendIndex, m_AudioVideoDataToSend);

				int tempILen = m_Tools.GetVideoBlockSizeFromMediaChunck(m_AudioVideoDataToSend);

				m_Tools.SetNumberOfAudioFramesInMediaChunck(LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_BLOCK_POSITION, vAudioDataLengthVector.size(), m_AudioVideoDataToSend);

				int index = LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_BLOCK_POSITION + LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_FRAME_BLOCK_SIZE;

//				LLG("#IV#S#     m_iAudioDataToSendIndex  = "+Tools::IntegertoStringConvert(m_iAudioDataToSendIndex));
//				LLG("#IV#S#     m_iDataToSendIndex  = "+Tools::IntegertoStringConvert(m_iDataToSendIndex));


				for (int i = 0; i < vAudioDataLengthVector.size(); i++)
				{
					m_Tools.SetNextAudioFramePositionInMediaChunck(index, vAudioDataLengthVector[i], m_AudioVideoDataToSend);
//					LLG("#IV#S#     AudiData  = "+Tools::IntegertoStringConvert(i)+"  ] = "+Tools::IntegertoStringConvert(vAudioDataLengthVector[i]));
					index += LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_SIZE;
				}

				m_Tools.SetNumberOfVideoFramesInMediaChunck(index, numberOfVideoPackets, m_AudioVideoDataToSend);

				index += LIVE_MEDIA_UNIT_NUMBER_OF_VIDEO_FRAME_BLOCK_SIZE;

				for (int i = 0; i < numberOfVideoPackets; i++)
				{
					m_Tools.SetNextAudioFramePositionInMediaChunck(index, videoPacketSizes[i], m_AudioVideoDataToSend);
//					LLG("#IV#S#     VideoData  = "+Tools::IntegertoStringConvert(i)+"  ] = "+Tools::IntegertoStringConvert(videoPacketSizes[i]));
					index += LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_SIZE;
				}

#ifndef NEW_HEADER_FORMAT

				for (int i = 1; i < NUMBER_OF_HEADER_FOR_STREAMING; i++)
					memcpy(m_AudioVideoDataToSend + i * packetSizeOfNetwork, m_AudioVideoDataToSend, packetSizeOfNetwork);

				index = packetSizeOfNetwork * NUMBER_OF_HEADER_FOR_STREAMING;

#endif

				memcpy(m_AudioVideoDataToSend + index, m_VideoDataToSend, m_iDataToSendIndex);
				memcpy(m_AudioVideoDataToSend + index + m_iDataToSendIndex, m_AudioDataToSend, m_iAudioDataToSendIndex);

				int tempILen2 = m_Tools.GetVideoBlockSizeFromMediaChunck(m_AudioVideoDataToSend);
                
                CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::SendingThreadProcedure() chunck ready");

				//LOGEF("THeKing--> sending --> iLen1 =  %d, iLen 2 = %d  [Video: %d   ,Audio: %d]\n", tempILen, tempILen2, m_iDataToSendIndex, m_iAudioDataToSendIndex);

				long long timeNow = packetHeader.getTimeStampDirectly(m_EncodedFrame + 1);

				int diff = timeNow - m_nTimeStampOfChunck;

				m_Tools.SetMediaUnitHeaderLengthInMediaChunck(index, m_AudioVideoDataToSend);
				m_Tools.SetMediaUnitChunkDurationInMediaChunck(diff, m_AudioVideoDataToSend);

#ifndef __LIVE_STREAMIN_SELF__

#ifdef NEW_HEADER_FORMAT

				if (m_bInterruptRunning == false)
				{
					if(m_bInterruptHappened == false)
#ifndef NO_CONNECTIVITY
						m_pCommonElementsBucket->SendFunctionPointer(index, 3, m_AudioVideoDataToSend, index + m_iDataToSendIndex + m_iAudioDataToSendIndex, diff);
#else
						m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(200, index + m_iDataToSendIndex + m_iAudioDataToSendIndex, m_AudioVideoDataToSend);
#endif
					else
						m_bInterruptHappened = false;
				}
				
#else
				if (m_bInterruptRunning == false)
				{
					if (m_bInterruptHappened == false)
						m_pCommonElementsBucket->SendFunctionPointer(m_pVideoCallSession->GetFriendID(), 3, m_AudioVideoDataToSend, packetSizeOfNetwork * NUMBER_OF_HEADER_FOR_STREAMING + m_iDataToSendIndex + m_iAudioDataToSendIndex, diff);
					else
						m_bInterruptHappened = false;
				}
				
#endif

				//m_pCommonElementsBucket->SendFunctionPointer(m_AudioDataToSend, m_iAudioDataToSendIndex, (int)llNowTimeDiff);
				//m_pCommonElementsBucket->SendFunctionPointer(m_VideoDataToSend, m_iDataToSendIndex, (int)llNowTimeDiff);


#else        
              /*  printf("Sending to liovestream, llNowTimeDiff = %lld\n", llNowTimeDiff);
                
                if(NULL != g_LiveReceiver)
                {
                    g_LiveReceiver->PushAudioData(m_AudioDataToSend, m_iAudioDataToSendIndex);
                    g_LiveReceiver->PushVideoData(m_VideoDataToSend, m_iDataToSendIndex);
                }*/

				//LOGEF("fahad -->> m_pCommonElementsBucket 3 --> lFriendID = 200, bExist = %d", bExist);

//				m_pVideoCallSession->m_pController->PushAudioForDecoding(m_lfriendID, m_AudioVideoDataToSend, index + m_iDataToSendIndex + m_iAudioDataToSendIndex);
//				if(bExist)
                
                int missingFrames[1003];
                int nMissingFrames = 0;

#ifdef	__RANDOM_MISSING_PACKET__

				int nTotalSizeToSend = packetSizeOfNetwork  * NUMBER_OF_HEADER_FOR_STREAMING  + m_iDataToSendIndex + m_iAudioDataToSendIndex;
				const int nMaxMissingFrames = (nTotalSizeToSend +  packetSizeOfNetwork - 1 ) / packetSizeOfNetwork;

                for(int i=0; i < nMaxMissingFrames; i ++)
                {
                    if(rand()%10 < 3)
                        missingFrames[nMissingFrames++] = i;
                }

#endif
                CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::SendingThreadProcedure() pushing for selfcall");
                //LOGEF("TheKing--> Processing LIVESTREAM\n");
				if(bExist)
				{

#ifdef NEW_HEADER_FORMAT

				if (m_bInterruptRunning == false)
				{
					if (m_bInterruptHappened == false)
						G_pInterfaceOfAudioVideoEngine->PushAudioForDecodingVector(m_pVideoCallSession->GetFriendID(), 3, m_AudioVideoDataToSend, index + m_iDataToSendIndex + m_iAudioDataToSendIndex, std::vector< std::pair<int, int> >());
					else
						m_bInterruptHappened = false;
				}
					
#else
				if (m_bInterruptRunning == false)
				{
					if (m_bInterruptHappened == false)
						G_pInterfaceOfAudioVideoEngine->PushAudioForDecoding(m_pVideoCallSession->GetFriendID(), 3, m_AudioVideoDataToSend, packetSizeOfNetwork * NUMBER_OF_HEADER_FOR_STREAMING + m_iDataToSendIndex + m_iAudioDataToSendIndex, nMissingFrames, missingFrames);
					else
						m_bInterruptHappened = false;
				}
					
#endif
				
				}
                CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::SendingThreadProcedure() pushed done");
#endif


                
                

				LOGEF("fahad (m_iDataToSendIndex,m_iAudioDataToSendIndex, total) -- ( %d, %d, %d )  --frameNumber == %d, bExist = %d %d %d %d\n", m_iDataToSendIndex, m_iAudioDataToSendIndex, m_iDataToSendIndex + m_iAudioDataToSendIndex, frameNumber, bExist, (int)llNowTimeDiff, diff, index);

				int tempIndex = m_iDataToSendIndex;
				numberOfVideoPackets = 0;
				m_iDataToSendIndex = 0;
				memcpy(m_VideoDataToSend + m_iDataToSendIndex ,m_EncodedFrame, packetSize);
				m_iDataToSendIndex += (packetSize);

				long long frameNumberHeader = packetHeader.GetFrameNumberDirectly(m_EncodedFrame + 1);

				m_nTimeStampOfChunck = packetHeader.getTimeStampDirectly(m_EncodedFrame + 1);

				//LOGEF("THeKing--> sending --> Video frameNumber = %d, frameNumberFromHeader = %d, FrameLength  = %d, iLen = %lld\n", frameNumber, frameNumberHeader, packetSize, tempIndex);

				videoPacketSizes[numberOfVideoPackets++] = packetSize;
			}
			else
			{
				if (firstFrame == true)
				{
					m_nTimeStampOfChunck = packetHeader.getTimeStampDirectly(m_EncodedFrame + 1);

					long long llNowLiveSendingTimeStamp = m_Tools.CurrentTimestamp();
					long long llNowTimeDiff;

					if (m_llPrevTimeWhileSendingToLive == 0)
					{
						llNowTimeDiff = 0;
						m_llPrevTimeWhileSendingToLive = llNowLiveSendingTimeStamp;
					}
					else
					{
						llNowTimeDiff = llNowLiveSendingTimeStamp - m_llPrevTimeWhileSendingToLive;
						m_llPrevTimeWhileSendingToLive = llNowLiveSendingTimeStamp;
					}
				}

                CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::SendingThreadProcedure() 200 ms not ready");
                
				if(m_iDataToSendIndex + packetSize < MAX_VIDEO_DATA_TO_SEND_SIZE)
				{  
					memcpy(m_VideoDataToSend + m_iDataToSendIndex ,m_EncodedFrame, packetSize);

					//CPacketHeader packetHeader;
					CVideoHeader packetHeader;
					long long frameNumberHeader = packetHeader.GetFrameNumberDirectly(m_EncodedFrame + 1);

					//LOGEF("THeKing--> sending --> Video frameNumber = %d, frameNumberFromHeader = %d, FrameLength  = %lld\n", frameNumber, frameNumberHeader, packetSize);
                    
                    unsigned char *p = m_VideoDataToSend+m_iDataToSendIndex + 1;
                    int nCurrentFrameLen = ((int)p[13] << 8) + p[14];
                    //CPacketHeader   ccc;
					CVideoHeader ccc;
                    ccc.setPacketHeader(p);
                    int nTemp = ccc.getPacketLength();
                    //printf("SendingSide--> nCurrentFrameLen = %d, but packetSize = %d, iDataToSendIndex = %d, gotLengthFromHeader = %d\n", nCurrentFrameLen, packetSize, m_iDataToSendIndex, nTemp); 
                    
					m_iDataToSendIndex += (packetSize);

					videoPacketSizes[numberOfVideoPackets++] = packetSize;
				}
			}
			firstFrame = false;
			toolsObject.SOSleep(0);
        }
else{	//packetHeader.setPacketHeader(m_EncodedFrame + 1);

			unsigned char signal = m_pVideoCallSession->GetFPSController()->GetFPSSignalByte();
			m_EncodedFrame[1 + 3] = signal;


#ifdef PACKET_SEND_STATISTICS_ENABLED

			int iNumberOfPackets = -1;

			iNumberOfPackets = packetHeader.getNumberOfPacket();

			pair<long long, int> FramePacketPair = /*toolsObject.GetFramePacketFromHeader(m_EncodedFrame + 1, iNumberOfPackets);*/make_pair(packetHeader.getFrameNumber(), packetHeader.getPacketNumber());

			if (FramePacketPair.first != iPrevFrameNumer)
			{
				//CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS,"iNumberOfPacketsActuallySentFromLastFrame = %d, iNumberOfPacketsInLastFrame = %d, currentframenumber = %d\n",
				//	iNumberOfPacketsActuallySentFromLastFrame, iNumberOfPacketsInLastFrame, FramePacketPair.first);

				if (iNumberOfPacketsActuallySentFromLastFrame != iNumberOfPacketsInLastFrame)
				{
					CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "CSendingThread::StartSendingThread() $$-->******* iNumberOfPacketsActuallySentFromLastFrame = "
						+ m_Tools.IntegertoStringConvert(iNumberOfPacketsActuallySentFromLastFrame)
						+ " iNumberOfPacketsInLastFrame = "
						+ m_Tools.IntegertoStringConvert(iNumberOfPacketsInLastFrame)
						+ " currentframenumber = "
						+ m_Tools.IntegertoStringConvert(FramePacketPair.first)
						+ " m_SendingBuffersize = "
						+ m_Tools.IntegertoStringConvert(m_SendingBuffer->GetQueueSize()));

				}


				iNumberOfPacketsInLastFrame = iNumberOfPackets;
				iNumberOfPacketsActuallySentFromLastFrame = 1;
				iPrevFrameNumer = FramePacketPair.first;
			}
			else
			{
				iNumberOfPacketsActuallySentFromLastFrame++;
			}
#endif


			//			CLogPrinter_WriteSpecific2(CLogPrinter::INFO, "CSendingThread::StartSendingThread() Parsing..>>>  FN: "+ m_Tools.IntegertoStringConvert(packetHeader.getFrameNumber())
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
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThread::SendingThreadProcedure() came for CALL !!!!!!!");

#ifndef NO_CONNECTIVITY
				m_pCommonElementsBucket->SendFunctionPointer(m_pVideoCallSession->GetFriendID(), 2, m_EncodedFrame, packetSize,0);
#else
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

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::SendingThreadProcedure() stopped SendingThreadProcedure method.");
}

void CSendingThread::InterruptOccured()
{
	m_bInterruptHappened = true;
	m_bInterruptRunning = true;
}

void CSendingThread::InterruptOver()
{
	m_bInterruptRunning = false;
}

int CSendingThread::GetSleepTime()
{
	int SleepTime = MAX_VIDEO_PACKET_SENDING_SLEEP_MS - (m_pVideoCallSession->GetVideoEncoder()->GetBitrate() / REQUIRED_BITRATE_FOR_UNIT_SLEEP);

	if(SleepTime < MIN_VIDEO_PACKET_SENDING_SLEEP_MS)
		SleepTime = MIN_VIDEO_PACKET_SENDING_SLEEP_MS;

	return SleepTime;
}
