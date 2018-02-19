
#include "SendingThread.h"
#include "Size.h"
//#include "PacketHeader.h"
#include "VideoHeader.h"
#include "CommonElementsBucket.h"
#include "VideoCallSession.h"
#include "Controller.h"
#include "AudioCallSession.h"

#include <vector>

#include "LiveReceiver.h"
#include "LiveVideoDecodingQueue.h"

#include "InterfaceOfAudioVideoEngine.h"

#ifdef CHANNEL_FROM_FILE
#include "Aac.h"
#endif

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

namespace MediaSDK
{
    
    extern CInterfaceOfAudioVideoEngine *G_pInterfaceOfAudioVideoEngine;
    
    //#define SEND_VIDEO_TO_SELF 1
    //#define __LIVE_STREAMIN_SELF__
    
    //#define __RANDOM_MISSING_PACKET__
    
    CSendingThread::CSendingThread(CCommonElementsBucket* commonElementsBucket, CSendingBuffer *sendingBuffer, CVideoCallSession* pVideoCallSession, bool bIsCheckCall, long long llfriendID, bool bAudioOnlyLive) :
    m_pCommonElementsBucket(commonElementsBucket),
    m_SendingBuffer(sendingBuffer),
    m_bIsCheckCall(bIsCheckCall),
    m_iAudioDataToSendIndex(0),
    m_nTimeStampOfChunck(-1),
    m_nTimeStampOfChunckSend(0),
    m_lfriendID(llfriendID),
    m_bInterruptHappened(false),
    m_bInterruptRunning(false),
    m_bResetForViewerCallerCallEnd(false),
    m_bResetForPublisherCallerCallStartAudioOnly(false),
    m_bAudioOnlyLive(bAudioOnlyLive),
    m_bVideoOnlyLive(false),
    m_bPassOnlyAudio(false),
    m_llBaseRelativeTimeOfAudio(-1),
	m_nChunkNumber(0)
    
    {
        m_pVideoCallSession = pVideoCallSession;
        
        if (pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM || pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL)
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
        CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThread::StartSendingThread() called");
        
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
        
        CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThread::StartSendingThread() Sending Thread started");
        
        return;
    }
    
    void *CSendingThread::CreateVideoSendingThread(void* param)
    {
        CSendingThread *pThis = (CSendingThread*)param;
        pThis->SendingThreadProcedure();
        
        return NULL;
    }
    
    void CSendingThread::ResetForViewerCallerCallEnd()
    {
        m_bResetForViewerCallerCallEnd = true;
        
        while (m_bResetForViewerCallerCallEnd)
        {
            m_Tools.SOSleep(5);
        }
    }
    
    void CSendingThread::ResetForPublisherCallerCallStartAudioOnly()
    {
        m_bResetForPublisherCallerCallStartAudioOnly = true;
        
        while (m_bResetForPublisherCallerCallStartAudioOnly)
        {
            m_Tools.SOSleep(5);
        }
    }
    
#ifdef PACKET_SEND_STATISTICS_ENABLED
    long long iPrevFrameNumer = 0;
    int iNumberOfPacketsInLastFrame = 0;
    int iNumberOfPacketsActuallySentFromLastFrame = 0;
#endif
    
#ifdef CHANNEL_FROM_FILE
    void CSendingThread::SendDataFromFile()
    {
        CVideoCallSession* pVideoSession;
        
        long long lFriendID = 200;
        //	std::string inFilePath = "sdcard/naac_file/chunks/chunk.";
        std::string inFilePath = "sdcard/test_files/chunks/chunk.";
        
        LOG_AAC("#aac#file# Sending File to AAC.");
        
        unsigned char data[300000];
        
        long long chunkDuration;
        long long lastSleepTime, curSleepTime;
        
        lastSleepTime = m_Tools.CurrentTimestamp();
        for (int i = 0; i <= 100; i++)
        {
            int totFileSize = -1;
            std::string filePath = inFilePath + m_Tools.IntegertoStringConvert(i);
            LOG_AAC("#aac#file# FilePath: %s", filePath.c_str());
            
            FILE *fd = fopen(filePath.c_str(), "rb");
            
            if (!fd){
                LOG_AAC("#aac#file# file open failed");
                return;
            }
            
            if (!fseek(fd, 0, SEEK_END))
            {
                totFileSize = ftell(fd);
                LOG_AAC("#aac#file# Reading from file: %lld", totFileSize);
            }
            
            fseek(fd, 0, SEEK_SET);
            fread(data, 1, totFileSize, fd);
            
            G_pInterfaceOfAudioVideoEngine->PushAudioForDecodingVector(lFriendID, MEDIA_TYPE_LIVE_STREAM, ENTITY_TYPE_VIEWER, data, totFileSize, std::vector< std::pair<int, int> >());
            fclose(fd);
            
            chunkDuration = m_Tools.GetMediaUnitChunkDurationFromMediaChunck(data);
            LOG_AAC("#aac#file# chunk_duration: %lld", chunkDuration);
            curSleepTime = m_Tools.CurrentTimestamp();
            m_Tools.SOSleep(chunkDuration - (curSleepTime - lastSleepTime));
            lastSleepTime = m_Tools.CurrentTimestamp();
            //		m_Tools.SOSleep(2000);
        }
    }
#endif
    
    void CSendingThread::SendingThreadProcedure()
    {
        CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThread::SendingThreadProcedure() started Sending method");
        
        Tools toolsObject;
        toolsObject.SetThreadName("SendingCommon");
		int packetSize = 0;
		long long lFriendID = m_lfriendID;
		int startFraction = SIZE_OF_INT_MINUS_8;
		int fractionInterval = BYTE_SIZE;
		int frameNumber, packetNumber;
		//CPacketHeader packetHeader;
		CVideoHeader packetHeader;
		std::vector<int> vAudioDataLengthVector;
		int videoPacketSizes[30];
		int numberOfVideoPackets = 0;
		int frameCounter = 0;
		int packetSizeOfMissing = 1000;

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
            //CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::SendingThreadProcedure() RUNNING Sending method");
#ifdef CHANNEL_FROM_FILE
            m_Tools.SOSleep(5000);
            SendDataFromFile();
            m_Tools.SOSleep(500000);
#endif
            
            if (m_bResetForViewerCallerCallEnd == true)
            {
                m_SendingBuffer->ResetBuffer();
                firstFrame = true;
				m_nChunkNumber = 0;
                
                m_bResetForViewerCallerCallEnd = false;
            }
            
            if (m_bResetForPublisherCallerCallStartAudioOnly == true)
            {
                firstFrame = true;
                
                m_bResetForPublisherCallerCallStartAudioOnly = false;
            }
            
            LOGSS("##SS## m_bAudioOnlyLive %d entityType %d callInLiveType %d ", m_bAudioOnlyLive, m_pVideoCallSession->GetEntityType(), m_pVideoCallSession->GetCallInLiveType());
            
            if (m_bAudioOnlyLive == true && m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_PUBLISHER_CALLER && (m_pVideoCallSession->GetCallInLiveType() == CALL_IN_LIVE_TYPE_AUDIO_VIDEO || m_pVideoCallSession->GetCallInLiveType() == CALL_IN_LIVE_TYPE_VIDEO_ONLY))
                m_bPassOnlyAudio = false;
            else if (m_bAudioOnlyLive == false && m_pVideoCallSession->GetEntityType() == ENTITY_TYPE_VIEWER_CALLEE && m_pVideoCallSession->GetCallInLiveType() == CALL_IN_LIVE_TYPE_AUDIO_ONLY)
                m_bPassOnlyAudio = true;
            else if (m_bAudioOnlyLive == true)
                m_bPassOnlyAudio = true;
            else
                m_bPassOnlyAudio = false;
            
            LOGSS("##SS## m_bPassOnlyAudio %d ", m_bPassOnlyAudio);

			CLogPrinter_LOG(LIVE_TYPE_LOG, "CSendingThread::SendingThreadProcedure m_bPassOnlyAudio %d m_bAudioOnlyLive %d entityType %d callInLiveType %d ", m_bPassOnlyAudio, m_bAudioOnlyLive, m_pVideoCallSession->GetEntityType(), m_pVideoCallSession->GetCallInLiveType());

			unsigned int uNumerOfFrames;
			if (m_bPassOnlyAudio == true)
			{
				CAudioCallSession *audioSession;
				int bCallExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, audioSession);
				if (bCallExist == false)
				{
					uNumerOfFrames = 0;
				}
				else
				{
					uNumerOfFrames = audioSession->GetNumberOfFrameForChunk();
				}
				MediaLog(LOG_DEBUG, "[ST] Audio Exist: %d, Frame Size: %u", bCallExist, uNumerOfFrames);
			}
            
            if (m_SendingBuffer->GetQueueSize() == 0 && m_bPassOnlyAudio == false)
            {
                CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThread::SendingThreadProcedure() NOTHING for Sending method");
                
                toolsObject.SOSleep(10);
            }
			else if (m_bPassOnlyAudio == true && uNumerOfFrames < 2)
			{
				toolsObject.SOSleep(10);
			}
			else if ((m_SendingBuffer->GetQueueSize() > 0 && m_bPassOnlyAudio == false) || (m_bPassOnlyAudio == true && uNumerOfFrames >= 2))
            {
                chunkStartTime = m_Tools.CurrentTimestamp();
                
                CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThread::SendingThreadProcedure() GOT packet for Sending method");
                
                int timeDiffForQueue = 0;
                
                if (m_bPassOnlyAudio == false)
                {
                    packetSize = m_SendingBuffer->DeQueue(lFriendID, m_EncodedFrame, frameNumber, packetNumber, timeDiffForQueue);
                }
                
                CLogPrinter_WriteLog(CLogPrinter::INFO, QUEUE_TIME_LOG, "CSendingThread::StartSendingThread() m_SendingBuffer " + toolsObject.IntegertoStringConvert(timeDiffForQueue));
                
                //printf("serverType Number %d\n", m_pVideoCallSession->GetServiceType());
                
                if ((m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL))
                {
                    CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThread::SendingThreadProcedure() session got");
                    
                    CVideoHeader temporaryHeader;
                    temporaryHeader.SetPacketHeader(m_EncodedFrame + 1);
                    
                    unsigned char *p = m_EncodedFrame + temporaryHeader.GetHeaderLength() + 1;
                    
                    int nalType = 0;
                    
                    if (packetSize > (temporaryHeader.GetHeaderLength() + 1) )
                        nalType = p[2] == 1 ? (p[3] & 0x1f) : (p[4] & 0x1f);
                    else
                        nalType = 0;
                    
                    CLogPrinter_LOG(CHUNK_SENDING_LOG, "CSendingThread::SendingThreadProcedure FFFFFFFFFFFFF Type %d size %d", nalType, packetSize);
                    
                    CAudioCallSession *pAudioSession;
                    
                    bool bExist = m_pCommonElementsBucket->m_pAudioCallSessionList->IsAudioSessionExist(lFriendID, pAudioSession);
                    
                    if (bExist && m_llBaseRelativeTimeOfAudio == -1)
                        m_llBaseRelativeTimeOfAudio = pAudioSession->GetBaseOfRelativeTime();
                    
                    LOGSS("##SS## firstFrame %d ", firstFrame);
                    
                    if (m_bPassOnlyAudio == true || (nalType == 7 && firstFrame == false))
                    {
                        CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThread::SendingThreadProcedure() 200 ms completed");
                        
                        //LOGEF("fahad -->> m_pCommonElementsBucket 2 --> lFriendID = %lld, bExist = %d", lFriendID, bExist);
                        
                        int viewerDataLength = 0, calleeDataLength = 0;
                        long long llAudioChunkDuration = 0, llAudioChunkRelativeTime = 0;
                        
                        m_iAudioDataToSendIndex = 0;
                        
                        if (vAudioDataLengthVector.size() > 0)
                            vAudioDataLengthVector.clear();
                        
                        HITLERSS("#RT### m_bAudioOnlyLive %d EntityType %d callInLiveType %d m_bPassOnlyAudio %d", m_bAudioOnlyLive, m_pVideoCallSession->GetEntityType(), m_pVideoCallSession->GetCallInLiveType(), m_bPassOnlyAudio);
                        
                        if (bExist && m_bVideoOnlyLive == false)
                        {
                            pAudioSession->GetAudioDataToSend(m_AudioDataToSend, m_iAudioDataToSendIndex, vAudioDataLengthVector, viewerDataLength, calleeDataLength, llAudioChunkDuration, llAudioChunkRelativeTime);
                            
                            if (m_bPassOnlyAudio == true)
                            {
                                /*
                                 if (m_bAudioOnlyDataAlreadySent == false && viewerDataLength <= 0)
                                 continue;
                                 else
                                 m_bAudioOnlyDataAlreadySent = true;
                                 */
                                
								if (m_iAudioDataToSendIndex <= 0)
                                    continue;
                            }
                        }
                        
                        HITLERSS("#RT### isAudioCallSessionExist: %d, audioChunkDuration: %lld, relativeTime: %lld, viewerDataLen: %d, calleeDataLen: %d", bExist, llAudioChunkDuration, llAudioChunkRelativeTime, viewerDataLength, calleeDataLength);
                        
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
                        
                        if (m_bPassOnlyAudio == true && m_nTimeStampOfChunck == -1)
                        {
                            m_nTimeStampOfChunck = llAudioChunkRelativeTime;
                        }
                        
                        m_nTimeStampOfChunckSend += llNowTimeDiff;
                        
                        //	m_Tools.IntToUnsignedCharConversion(m_iDataToSendIndex, m_AudioVideoDataToSend, 0);
                        //	m_Tools.IntToUnsignedCharConversion(m_iAudioDataToSendIndex, m_AudioVideoDataToSend, 4);
                        
                        m_Tools.SetMediaUnitVersionInMediaChunck(LIVE_HEADER_VERSION, m_AudioVideoDataToSend);
                        m_Tools.SetMediaUnitTimestampInMediaChunck(m_nTimeStampOfChunck, m_AudioVideoDataToSend);
                        m_Tools.SetAudioBlockSizeInMediaChunck(m_iAudioDataToSendIndex, m_AudioVideoDataToSend);
                        
                        if (m_bPassOnlyAudio)
                        {
                            m_iDataToSendIndex = 0;
                        }
                        
#ifdef DISABLE_VIDEO_FOR_LIVE
                        
                        m_iDataToSendIndex = 0;
#endif
                        
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
                        
                        if (m_bPassOnlyAudio)
                        {
                            numberOfVideoPackets = 0;
                        }
                        
#ifdef DISABLE_VIDEO_FOR_LIVE
                        
                        numberOfVideoPackets = 0;
#endif
                        m_Tools.SetNumberOfVideoFramesInMediaChunck(index, numberOfVideoPackets, m_AudioVideoDataToSend);
                        
                        index += LIVE_MEDIA_UNIT_NUMBER_OF_VIDEO_FRAME_BLOCK_SIZE;
                        
#ifndef DISABLE_VIDEO_FOR_LIVE
                        
                        if (m_bPassOnlyAudio == false)
                        {
                            for (int i = 0; i < numberOfVideoPackets; i++)
                            {
                                m_Tools.SetNextAudioFramePositionInMediaChunck(index, videoPacketSizes[i], m_AudioVideoDataToSend);
                                //					LLG("#IV#S#     VideoData  = "+Tools::IntegertoStringConvert(i)+"  ] = "+Tools::IntegertoStringConvert(videoPacketSizes[i]));
                                index += LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_SIZE;
                            }
                        }
#endif

						m_Tools.SetAudioBlockStartingPositionInMediaChunck(index + m_iDataToSendIndex, m_AudioVideoDataToSend);
						m_Tools.SetVideoBlockStartingPositionInMediaChunck(index, m_AudioVideoDataToSend);

						LOGE("audioStartingPosition %d videoStartingPosition %d\n", index + m_iDataToSendIndex, index);

#ifndef DISABLE_VIDEO_FOR_LIVE
                        
                        if (m_bPassOnlyAudio == false)
                        {
                            memcpy(m_AudioVideoDataToSend + index, m_VideoDataToSend, m_iDataToSendIndex);
                        }
#endif
                        memcpy(m_AudioVideoDataToSend + index + m_iDataToSendIndex, m_AudioDataToSend, m_iAudioDataToSendIndex);
                        
                        int tempILen2 = m_Tools.GetVideoBlockSizeFromMediaChunck(m_AudioVideoDataToSend);
                        
                        CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThread::SendingThreadProcedure() chunck ready");
                        
                        //LOGEF("THeKing--> sending --> iLen1 =  %d, iLen 2 = %d  [Video: %d   ,Audio: %d]\n", tempILen, tempILen2, m_iDataToSendIndex, m_iAudioDataToSendIndex);
                        
                        long long timeNow;
                        long long thisFrameTime = m_Tools.CurrentTimestamp();
                        
                        if (m_bPassOnlyAudio == true)
                        {
                            timeNow = llAudioChunkRelativeTime;
                        }
                        else
                        {
                            timeNow = packetHeader.GetTimeStampDirectly(m_EncodedFrame + 1);
                            
                            if (m_bAudioOnlyLive)
                            {
                                timeNow = thisFrameTime - m_llBaseRelativeTimeOfAudio;
                            }
                        }
                        
                        int diff = (int)(timeNow - m_nTimeStampOfChunck);
                        
                        LOGSS("##SS## even m_nTimeStampOfChunck %lld diff %d", m_nTimeStampOfChunck, diff);
                        
                        m_Tools.SetMediaUnitHeaderLengthInMediaChunck(index, m_AudioVideoDataToSend);
                        m_Tools.SetMediaUnitStreamTypeInMediaChunck(STREAM_TYPE_LIVE_STREAM, m_AudioVideoDataToSend);
                        m_Tools.SetMediaUnitBlockInfoPositionInMediaChunck(LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_BLOCK_POSITION, m_AudioVideoDataToSend);
                        m_Tools.SetMediaUnitChunkDurationInMediaChunck(diff, m_AudioVideoDataToSend);

						m_Tools.SetEntityTypeInMediaChunck(m_pVideoCallSession->GetEntityType(), m_AudioVideoDataToSend);
						m_Tools.SetServiceTypeInMediaChunck(m_pVideoCallSession->GetServiceType(), m_AudioVideoDataToSend);
						m_Tools.SetMediaUnitChunkNumberInMediaChunck(m_nChunkNumber, m_AudioVideoDataToSend);

						m_nChunkNumber++;
                        
                        HITLERSS("#RT### Sending 0");
                        
#ifndef __LIVE_STREAMIN_SELF__

						HITLERSS("#RT### Sending 01");

						if (m_bInterruptRunning == false)
						{
							if (m_bInterruptHappened == false)
							{
#ifndef NO_CONNECTIVITY
                                
                                if (m_bVideoOnlyLive == false)
                                {
                                    
                                    HITLERSS("#RT### Sending 02");
                                    
                                    HITLER("#@#@26022017# SENDING DATA WITH LENGTH = %d", index + m_iDataToSendIndex + m_iAudioDataToSendIndex);
                                    
                                    int viewerDataIndex = index + m_iDataToSendIndex;	/*Audio Data Start Index.*/
                                    int calleeDataIndex = viewerDataIndex + viewerDataLength;
                                    
                                    std::vector<std::pair<int, int> > liVector;
                                    
									MediaLog(LOG_DEBUG, "[NE][ST] AudioStartInd:%d, TotalAudio:%d[%d+%d]", viewerDataIndex, m_iAudioDataToSendIndex, viewerDataLength, calleeDataLength);
									//liVector.push_back(std::make_pair(viewerDataIndex, viewerDataLength));									
									liVector.push_back(std::make_pair(viewerDataIndex, m_iAudioDataToSendIndex));
                                    liVector.push_back(std::make_pair(0, 0));
                                                                        
                                    
                                    //if (ENTITY_TYPE_VIEWER_CALLEE == m_pVideoCallSession->GetEntityType())
                                    //{
                                    //	reverse(liVector.begin(), liVector.end());	//Callee Data, Viewer data.
                                    //}
                                    
                                    // do changes for audio
                                    if ((diff > 1000 && m_bPassOnlyAudio == true) || diff < 0)
                                    {
                                        
                                    }
                                    else
                                    {
										for (int i = 0; i < vAudioDataLengthVector.size(); i++)
										{
											if (i == 0)
											{
												pAudioSession->m_pChunckedNE->WriteDump(m_AudioVideoDataToSend + index + m_iDataToSendIndex + 21, 1, vAudioDataLengthVector[0]);
											}
											else
											{
												pAudioSession->m_pChunckedNE->WriteDump(m_AudioVideoDataToSend + index + m_iDataToSendIndex + vAudioDataLengthVector[i - 1] + 21, 1, vAudioDataLengthVector[i]);
											}
										}

                                        CLogPrinter_LOG(CHUNK_SENDING_LOG, "CSendingThread::SendingThreadProcedure sending chunk size %d duration %d", index + m_iDataToSendIndex + m_iAudioDataToSendIndex, diff);
                                        
                                        m_pCommonElementsBucket->SendFunctionPointer(index, MEDIA_TYPE_LIVE_STREAM, m_AudioVideoDataToSend, index + m_iDataToSendIndex + m_iAudioDataToSendIndex, diff, liVector);
										//this->ParseChunk(m_AudioVideoDataToSend, index + m_iDataToSendIndex + m_iAudioDataToSendIndex,"ST");
                                    }
                                    //LOGT("##TN##CALLBACK## viewerdataindex:%d viewerdatalength:%d || calleedataindex:%d calleedatalength:%d", viewerDataIndex, viewerDataLength, calleeDataIndex, calleeDataLength);
                                    
                                }
#else
                                HITLER("#@#@26022017# SENDING DATA WITH LENGTH = %d", index + m_iDataToSendIndex + m_iAudioDataToSendIndex);
                                printf("TheKing--> SendingSide TimeStampOfChunk %lld\n",m_nTimeStampOfChunck);
                                this->ParseChunk(m_AudioVideoDataToSend, index + m_iDataToSendIndex + m_iAudioDataToSendIndex,"1");
                                m_pCommonElementsBucket->m_pEventNotifier->fireAudioPacketEvent(200, index + m_iDataToSendIndex + m_iAudioDataToSendIndex, m_AudioVideoDataToSend);
#endif
							}
							else
							{
								HITLERSS("#RT### Sending 03");
								m_bInterruptHappened = false;
							}
						}

						//m_pCommonElementsBucket->SendFunctionPointer(m_AudioDataToSend, m_iAudioDataToSendIndex, (int)llNowTimeDiff);
						//m_pCommonElementsBucket->SendFunctionPointer(m_VideoDataToSend, m_iDataToSendIndex, (int)llNowTimeDiff);


#else        
						HITLERSS("#RT### Sending 11");
						/*  printf("Sending to liovestream, llNowTimeDiff = %lld\n", llNowTimeDiff);

						  if(NULL != g_LiveReceiver)
						  {                    
						  g_LiveReceiver->PushVideoData(m_VideoDataToSend, m_iDataToSendIndex);
						  }*/

						//LOGEF("fahad -->> m_pCommonElementsBucket 3 --> lFriendID = 200, bExist = %d", bExist);

						//				m_pVideoCallSession->m_pController->PushAudioForDecoding(m_lfriendID, m_AudioVideoDataToSend, index + m_iDataToSendIndex + m_iAudioDataToSendIndex);
						//				if(bExist)

						int missingFrames[1003];
						int nMissingFrames = 0;

#ifdef	__RANDOM_MISSING_PACKET__

						int nTotalSizeToSend = packetSizeOfMissing  * NUMBER_OF_HEADER_FOR_STREAMING  + m_iDataToSendIndex + m_iAudioDataToSendIndex;
						const int nMaxMissingFrames = (nTotalSizeToSend +  packetSizeOfMissing - 1 ) / packetSizeOfMissing;

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
							if (m_bInterruptRunning == false)
							{
								if (m_bInterruptHappened == false)
									G_pInterfaceOfAudioVideoEngine->PushAudioForDecodingVector(m_pVideoCallSession->GetFriendID(), MEDIA_TYPE_LIVE_STREAM,  m_pVideoCallSession->GetEntityType(),  m_AudioVideoDataToSend, index + m_iDataToSendIndex + m_iAudioDataToSendIndex, std::vector< std::pair<int, int> >());
								else
									m_bInterruptHappened = false;
							}
						}

						CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CSendingThread::SendingThreadProcedure() pushed done");
#endif
                        
                        LOGEF("fahad (m_iDataToSendIndex,m_iAudioDataToSendIndex, total) -- ( %d, %d, %d )  --frameNumber == %d, bExist = %d %d %d %d\n", m_iDataToSendIndex, m_iAudioDataToSendIndex, m_iDataToSendIndex + m_iAudioDataToSendIndex, frameNumber, bExist, (int)llNowTimeDiff, diff, index);
                        
                        int tempIndex = m_iDataToSendIndex;
                        numberOfVideoPackets = 0;
                        
						m_VideoDataToSend[0] = (unsigned char)0;
						m_VideoDataToSend[1] = (unsigned char)0;
						m_VideoDataToSend[2] = (unsigned char)0;

						m_iDataToSendIndex = 3;

                        memcpy(m_VideoDataToSend + m_iDataToSendIndex, m_EncodedFrame, packetSize);
                        m_iDataToSendIndex += (packetSize);
                        
                        CLogPrinter_LOG(CHUNK_SENDING_LOG, "CSendingThread::SendingThreadProcedure FIRSTTTT frame Type %d size %d", nalType, packetSize);
                        
                        long long frameNumberHeader = packetHeader.GetFrameNumberDirectly(m_EncodedFrame + 1);
                        
                        if (m_bPassOnlyAudio == true)
                        {
                            m_nTimeStampOfChunck = llAudioChunkRelativeTime;
                        }
                        else if (m_bAudioOnlyLive)
                        {
                            m_nTimeStampOfChunck = thisFrameTime - m_llBaseRelativeTimeOfAudio;
                        }
                        else
                        {
                            m_nTimeStampOfChunck = packetHeader.GetTimeStampDirectly(m_EncodedFrame + 1);
                        }
                        
                        LOGSS("##SS## odd m_nTimeStampOfChunck %lld", m_nTimeStampOfChunck);
                        
                        //LOGEF("THeKing--> sending --> Video frameNumber = %d, frameNumberFromHeader = %d, FrameLength  = %d, iLen = %lld\n", frameNumber, frameNumberHeader, packetSize, tempIndex);
                        
                        videoPacketSizes[numberOfVideoPackets++] = packetSize;
                    }
                    else
                    {
                        if (firstFrame == true)
                        {
                            m_nTimeStampOfChunck = packetHeader.GetTimeStampDirectly(m_EncodedFrame + 1);
                            
                            if (m_bAudioOnlyLive)
                            {
                                m_nTimeStampOfChunck = m_Tools.CurrentTimestamp() - m_llBaseRelativeTimeOfAudio;
                            }
                            
                            LOGSS("##SS## first m_nTimeStampOfChunck %lld", m_nTimeStampOfChunck);
                            
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

							numberOfVideoPackets = 0;

							m_VideoDataToSend[0] = (unsigned char)0;
							m_VideoDataToSend[1] = (unsigned char)0;
							m_VideoDataToSend[2] = (unsigned char)0;

							m_iDataToSendIndex = 3;
                        }
                        
                        CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThread::SendingThreadProcedure() 200 ms not ready");
                        
                        if (firstFrame == true)
                            CLogPrinter_LOG(CHUNK_SENDING_LOG, "CSendingThread::SendingThreadProcedure FIRSTTTT frame Type %d size %d", nalType, packetSize);
                        else
                            CLogPrinter_LOG(CHUNK_SENDING_LOG, "CSendingThread::SendingThreadProcedure frame Type %d size %d", nalType, packetSize);
                        
                        if (m_iDataToSendIndex + packetSize < MAX_VIDEO_DATA_TO_SEND_SIZE)
                        {
                            memcpy(m_VideoDataToSend + m_iDataToSendIndex, m_EncodedFrame, packetSize);
                            
                            //CPacketHeader packetHeader;
                            //CVideoHeader packetHeader;
                            //long long frameNumberHeader = packetHeader.GetFrameNumberDirectly(m_EncodedFrame + 1);
                            
                            //LOGEF("THeKing--> sending --> Video frameNumber = %d, frameNumberFromHeader = %d, FrameLength  = %lld\n", frameNumber, frameNumberHeader, packetSize);
                            
                           // unsigned char *p = m_VideoDataToSend + m_iDataToSendIndex + 1;
                            //int nCurrentFrameLen = ((int)p[13] << 8) + p[14];
                            //CPacketHeader   ccc;
                            //CVideoHeader ccc;
                            //ccc.SetPacketHeader(p);
                            //int nTemp = ccc.GetPacketLength();
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
                    
                    if (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL)
                    {
                        m_EncodedFrame[2 + 3] = signal;
                    }
                    else
                    {
                        m_EncodedFrame[1 + 3] = signal;
                    }
                    
#ifdef PACKET_SEND_STATISTICS_ENABLED
                    
                    int iNumberOfPackets = -1;
                    
                    iNumberOfPackets = packetHeader.GetNumberOfPacket();
                    
                    pair<long long, int> FramePacketPair = /*toolsObject.GetFramePacketFromHeader(m_EncodedFrame + 1, iNumberOfPackets);*/make_pair(packetHeader.GetFrameNumber(), packetHeader.GetPacketNumber());
                    
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
                            HITLER("#@#@26022017# SENDING DATA WITH LENGTH = %d", packetSize);
                            if (m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_LIVE_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_SELF_STREAM || m_pVideoCallSession->GetServiceType() == SERVICE_TYPE_CHANNEL)
                            {
                                m_pCommonElementsBucket->SendFunctionPointer(m_pVideoCallSession->GetFriendID(), MEDIA_TYPE_LIVE_CALL_VIDEO, m_EncodedFrame, packetSize, 0, std::vector< std::pair<int, int> >());
                            }
                            else
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
                        toolsObject.SOSleep(1);
                        
#ifdef  BANDWIDTH_CONTROLLING_TEST
                    }
#endif
                }
                
                
            }
        }
        
        bSendingThreadClosed = true;
        
        CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG, "CSendingThread::SendingThreadProcedure() stopped SendingThreadProcedure method.");
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
        
        if (SleepTime < MIN_VIDEO_PACKET_SENDING_SLEEP_MS)
            SleepTime = MIN_VIDEO_PACKET_SENDING_SLEEP_MS;
        
        return SleepTime;
    }
    
	int CSendingThread::ParseChunk(unsigned char *in_data, unsigned int unLength, std::string tag)
    {
        printf("SendingSide DATA FOR BOKKOR %u\n", unLength);
        
        int nValidHeaderOffset = 0;
		Tools m_Tools;
		long long itIsNow = m_Tools.CurrentTimestamp();
        long long recvTimeOffset = m_Tools.GetMediaUnitTimestampInMediaChunck(in_data + nValidHeaderOffset);
        
        //LOGE("##DE#Interface## now %lld peertimestamp %lld timediff %lld relativediff %lld", itIsNow, recvTimeOffset, itIsNow - m_llTimeOffset, recvTimeOffset);
        
        
        long long expectedTime = itIsNow;
        int tmp_headerLength = m_Tools.GetMediaUnitHeaderLengthFromMediaChunck(in_data + nValidHeaderOffset);
        int tmp_chunkDuration = m_Tools.GetMediaUnitChunkDurationFromMediaChunck(in_data + nValidHeaderOffset);
        printf("SendingSide now:%lld peertimestamp:%lld expected:%lld  [%lld] CHUNK_DURA = %d HEAD_LEN = %d\n", itIsNow, recvTimeOffset, expectedTime, recvTimeOffset - expectedTime, tmp_chunkDuration, tmp_headerLength);
        
        
        
        printf("SendingSide recvTimeOffset  %lld\n", recvTimeOffset);
        
        int version = m_Tools.GetMediaUnitVersionFromMediaChunck(in_data + nValidHeaderOffset);
        
        int headerLength = m_Tools.GetMediaUnitHeaderLengthFromMediaChunck(in_data + nValidHeaderOffset);
        int chunkDuration = m_Tools.GetMediaUnitChunkDurationFromMediaChunck(in_data + nValidHeaderOffset);
        
        printf("SendingSide--> headerLength %d chunkDuration %d\n", headerLength, chunkDuration);
        
        int lengthOfAudioData = m_Tools.GetAudioBlockSizeFromMediaChunck(in_data + nValidHeaderOffset);
        int lengthOfVideoData = m_Tools.GetVideoBlockSizeFromMediaChunck(in_data + nValidHeaderOffset);
        
        printf("SendingSide interface:receive ############## lengthOfVideoData =  %d  lengthOfAudiooData = %d Offset= %d,  \n", lengthOfVideoData, lengthOfAudioData, nValidHeaderOffset);
        
        
        int audioFrameSizes[200];
        int videoFrameSizes[150];
        
        int blockInfoPosition = m_Tools.GetMediaUnitBlockInfoPositionFromMediaChunck(in_data + nValidHeaderOffset);
        
        int numberOfAudioFrames = m_Tools.GetNumberOfAudioFramesFromMediaChunck(blockInfoPosition, in_data + nValidHeaderOffset);
        
        int index = blockInfoPosition + LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_FRAME_BLOCK_SIZE;
        
        for (int i = 0; i < numberOfAudioFrames; i++)
        {
            audioFrameSizes[i] = m_Tools.GetNextAudioFramePositionFromMediaChunck(index, in_data + nValidHeaderOffset);
            
            index += LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_SIZE;
        }
        
        int numberOfVideoFrames = m_Tools.GetNumberOfVideoFramesFromMediaChunck(index, in_data + nValidHeaderOffset);
        
        index += LIVE_MEDIA_UNIT_NUMBER_OF_VIDEO_FRAME_BLOCK_SIZE;
        
        for (int i = 0; i < numberOfVideoFrames; i++)
        {
            videoFrameSizes[i] = m_Tools.GetNextAudioFramePositionFromMediaChunck(index, in_data + nValidHeaderOffset);
            
            index += LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_SIZE;
        }
        
        printf("SendingSide GotNumberOfAudioFrames: %d, numberOfVideoFrames: %d,  audioDataSize: %d", numberOfAudioFrames, numberOfVideoFrames, lengthOfAudioData);
        
        int audioStartingPosition = m_Tools.GetAudioBlockStartingPositionFromMediaChunck(in_data + nValidHeaderOffset);
        int videoStartingPosition = m_Tools.GetVideoBlockStartingPositionFromMediaChunck(in_data + nValidHeaderOffset);
        int streamType = m_Tools.GetMediaUnitStreamTypeFromMediaChunck(in_data + nValidHeaderOffset);
		int ind = audioStartingPosition;

		
		MediaLog(LOG_CODE_TRACE, "[%s] -------------->HL:%d ASI:%d ADL: %d, AFrames: %d RT:%lld[%d]\n", 
			tag.c_str(),headerLength, audioStartingPosition, lengthOfAudioData, numberOfAudioFrames, recvTimeOffset, tmp_chunkDuration);

		for (int i = 0; i < numberOfAudioFrames; i++)
		{
			MediaLog(LOG_CODE_TRACE, "[%s] %d-> PT:%d, L:%d FrmSI:%d\n",tag.c_str(), i, (int)in_data[ind + 1], audioFrameSizes[i], ind);
			ind += audioFrameSizes[i];
		}

        printf("SendingSide audioStartingPosition = %d, videoStartingPosition = %d, streamType = %d\n", audioStartingPosition, videoStartingPosition, streamType);
        
        return 0;
    }
    
} //namespace
