//
// Created by ipvision on 10/23/2016.
//

#include "LiveReceiver.h"
//#include "PacketHeader.h"
#include "VideoHeader.h"
#include "Tools.h"
#include "ThreadTools.h"
#include "CommonElementsBucket.h"
#include "AudioPacketHeader.h"

LiveReceiver::LiveReceiver(CCommonElementsBucket* sharedObject, CAudioCallSession* pAudioCallSession) :
m_pCommonElementsBucket(sharedObject),
m_pAudioCallSession(pAudioCallSession)
{
    //m_pAudioDecoderBuffer = pAudioDecoderBuffer;
    m_pLiveAudioReceivedQueue = NULL;
    m_pLiveVideoDecodingQueue = NULL;
    m_pLiveReceiverMutex.reset(new CLockHandler);
	m_bIsCurrentlyParsingAudioData = false;
	m_bIsRoleChanging = false;

	m_pAudioPacketHeader = new CAudioPacketHeader();

	// logFile = fopen("/sdcard/LiveAudioMissing.txt", "w");
}

LiveReceiver::~LiveReceiver(){
    SHARED_PTR_DELETE(m_pLiveReceiverMutex);
	delete m_pAudioPacketHeader;
	// fclose(logFile);
}
void LiveReceiver::SetVideoDecodingQueue(LiveVideoDecodingQueue *pQueue)
{
    m_pLiveVideoDecodingQueue = pQueue;
}
void LiveReceiver::SetAudioDecodingQueue(LiveAudioDecodingQueue *pQueue)
{
    m_pLiveAudioReceivedQueue = pQueue;
}


void LiveReceiver::PushVideoData(unsigned char* uchVideoData, int iLen, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames)
{
    Locker lock(*m_pLiveReceiverMutex);
    int iUsedLen = 0, nFrames = 0;
	int packetSizeOfNetwork = m_pCommonElementsBucket->GetPacketSizeOfNetwork();
	int offset = packetSizeOfNetwork * NUMBER_OF_HEADER_FOR_STREAMING;
	int tillIndex = offset;
    //int frameCounter = 0;

	if (packetSizeOfNetwork < 0)
		return;

	for (int j = 0; iUsedLen < iLen;j++)
    {
        nFrames++;

		int indexOfThisFrame = tillIndex;
		int endOfThisFrame = indexOfThisFrame + frameSizes[j] - 1;
		int commonLeft, commonRight;
		bool bBroken = false;

//        LOGEF("THeKing--> ^^^^^^^^^^^^^^^^^^^ LiveReceiver::PushVideoData :: 1");



		for (int i = 0; i < numberOfMissingFrames; i++)
		{
			commonLeft = max(indexOfThisFrame, missingFrames[i] * packetSizeOfNetwork);
			commonRight = min(endOfThisFrame,( (missingFrames[i] + 1) * packetSizeOfNetwork) - 1);

			if (commonLeft <= commonRight)
			{
				bBroken = true;
				break;
			}
		}

//        LOGEF("THeKing--> ^^^^^^^^^^^^^^^^^^^ LiveReceiver::PushVideoData :: 2");

		if (bBroken && j == 0)	//If I frame is missing.
			return;
		
		
		/*
		if (j == 0)
		{
			if (0 < numberOfMissingFrames &&  endOfThisFrame >= missingFrames[0] * packetSizeOfNetwork && missingFrames[0] >= NUMBER_OF_HEADER_FOR_STREAMING)
				return;
		}
		else
		{
			for (int i = 0; i < numberOfMissingFrames-1; i++)
			{
				if (endOfThisFrame >= missingFrames[i] * packetSizeOfNetwork && endOfThisFrame < missingFrames[i+1] * packetSizeOfNetwork)
					continue;
			}

			if (numberOfMissingFrames > 0 && endOfThisFrame >= missingFrames[numberOfMissingFrames - 1] * packetSizeOfNetwork && endOfThisFrame < (iLen + offset))
				continue;
		}	
		*/
//      packetHeaderObj.setPacketHeader(uchVideoData + iUsedLen);

		if (!bBroken)	//If the current frame is not broken.
		{
//            LOGEF("THeKing--> ^^^^^^^^^^^^^^^^^^^ LiveReceiver::PushVideoData :: 3 - before access data");
			int nCurrentFrameLen = ((int)uchVideoData[1 + iUsedLen + 13] << 8) + uchVideoData[1 + iUsedLen + 14];
            
			//LLG("#IV#    LiveReceiver::PushVideoData , nCurrentFrameLen = " + Tools::IntegertoStringConvert(nCurrentFrameLen));
//            LOGEF("THeKing--> ^^^^^^^^^^^^^^^^^^^ LiveReceiver::PushVideoData :: 3 - get current frame");

            //int frameNumber = packetHeader.GetFrameNumberDirectly(uchVideoData + (iUsedLen +1) );
			//LOGEF("THeKing--> receive #####  [%d] Video FrameCounter = %d, FrameLength  = %d, UsedLen: %d iLen = %d\n",j, frameNumber, nCurrentFrameLen + PACKET_HEADER_LENGTH_WITH_MEDIA_TYPE, iUsedLen, iLen);

			m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen + 1, nCurrentFrameLen + PACKET_HEADER_LENGTH);
//			iUsedLen += nCurrentFrameLen + PACKET_HEADER_LENGTH + 1;
			//iUsedLen += LIVE_STREAMING_PACKETIZATION_PACKET_SIZE * ((nCurrentFrameLen + PACKET_HEADER_LENGTH + 1 + LIVE_STREAMING_PACKETIZATION_PACKET_SIZE - 1) / LIVE_STREAMING_PACKETIZATION_PACKET_SIZE);
		}
        else
        {
            //LOGEF("THeKing--> receive #####  [%d] Broken## UsedLen: %d iLen = %d\n",j, iUsedLen, iLen);
        }


        iUsedLen += frameSizes[j];
		tillIndex = endOfThisFrame + 1;
	}

//    m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen, iLen + PACKET_HEADER_LENGTH);
}

bool LiveReceiver::GetVideoFrame(unsigned char* uchVideoFrame,int iLen)
{
    Locker lock(*m_pLiveReceiverMutex);
    return false;
}

void LiveReceiver::PushVideoDataVector(int offset, unsigned char* uchVideoData, int iLen, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
{
	Locker lock(*m_pLiveReceiverMutex);
	int numberOfMissingFrames = vMissingFrames.size();
	int numOfMissingFrames = 0;

	int iUsedLen = 0, nFrames = 0;
	int tillIndex = offset;
	//int frameCounter = 0;

	for (int j = 0; iUsedLen < iLen; j++)
	{
		nFrames++;

		int indexOfThisFrame = tillIndex;
		int endOfThisFrame = indexOfThisFrame + frameSizes[j] - 1;
		int commonLeft, commonRight;
		bool bBroken = false;

		//        LOGEF("THeKing--> ^^^^^^^^^^^^^^^^^^^ LiveReceiver::PushVideoData :: 1");



		for (int i = 0; i < numberOfMissingFrames; i++)
		{
			commonLeft = max(indexOfThisFrame, vMissingFrames[i].first);
			commonRight = min(endOfThisFrame, vMissingFrames[i].second);

			if (commonLeft <= commonRight)
			{
				bBroken = true;
				break;
			}
		}

		//        LOGEF("THeKing--> ^^^^^^^^^^^^^^^^^^^ LiveReceiver::PushVideoData :: 2");

		if (bBroken && j == 0)	//If I frame is missing.
		{
			LOG_AAC("#aac#b4q# Missing iFrames: %d", frameSizes[0]);
			CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::PushVideoDataVector video frome broken j = " + m_Tools.getText(j) + " size " + m_Tools.getText(frameSizes[j]));

			return;
		}


		/*
		if (j == 0)
		{
		if (0 < numberOfMissingFrames &&  endOfThisFrame >= missingFrames[0] * packetSizeOfNetwork && missingFrames[0] >= NUMBER_OF_HEADER_FOR_STREAMING)
		return;
		}
		else
		{
		for (int i = 0; i < numberOfMissingFrames-1; i++)
		{
		if (endOfThisFrame >= missingFrames[i] * packetSizeOfNetwork && endOfThisFrame < missingFrames[i+1] * packetSizeOfNetwork)
		continue;
		}

		if (numberOfMissingFrames > 0 && endOfThisFrame >= missingFrames[numberOfMissingFrames - 1] * packetSizeOfNetwork && endOfThisFrame < (iLen + offset))
		continue;
		}
		*/
		//      packetHeaderObj.setPacketHeader(uchVideoData + iUsedLen);

		if (!bBroken)	//If the current frame is not broken.
		{
			//int nCurrentFrameLen = ((int)uchVideoData[1 + iUsedLen + 13] << 8) + uchVideoData[1 + iUsedLen + 14];
            
            CVideoHeader videoHeader;
            videoHeader.setPacketHeader(uchVideoData + 1 + iUsedLen);
            int nCurrentFrameLen = videoHeader.getPacketLength();

			unsigned char *p = uchVideoData + iUsedLen + 1 + videoHeader.GetHeaderLength();

			int nalType = p[2] == 1 ? (p[3] & 0x1f) : (p[4] & 0x1f);
			
			if (nalType == 7) 
				CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::PushVideoDataVector() found frome j = " + m_Tools.getText(j) + " size " + m_Tools.getText(frameSizes[j]));
			else
				CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::PushVideoDataVector() found frame j = " + m_Tools.getText(j) + " size " + m_Tools.getText(frameSizes[j]));

			//m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen + 1, nCurrentFrameLen + PACKET_HEADER_LENGTH);
            m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen + 1, nCurrentFrameLen + videoHeader.GetHeaderLength());
		}
		else
		{
			numOfMissingFrames++;
			CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::PushVideoDataVector video frame broken j = " + m_Tools.getText(j) + " size " + m_Tools.getText(frameSizes[j]));
			//LOGEF("THeKing--> receive #####  [%d] Broken## UsedLen: %d iLen = %d\n",j, iUsedLen, iLen);
		}


		iUsedLen += frameSizes[j];
		tillIndex = endOfThisFrame + 1;
	}

	LOG_AAC("#aac#b4q# TotalVideoFrames: %d, PushedVideoFrames: %d, NumOfMissingVideoFrames: %d", numberOfFrames, (numberOfFrames-numOfMissingFrames), numOfMissingFrames);
	//    m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen, iLen + PACKET_HEADER_LENGTH);
}


void LiveReceiver::ProcessAudioStream(int nOffset, unsigned char* uchAudioData, int nDataLength, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, std::vector< std::pair<int,int> > vMissingBlocks)
{
	if (m_bIsRoleChanging)
	{
		//LOGE("###DE### role changin lr....");
		return;
	}

	for (auto &missing : vMissingBlocks)
	{
		HITLER("XXP@#@#MARUF LIVE ST %d ED %d", missing.first, missing.second);
		// fprintf(logFile, "XXP@#@#MARUF LIVE ST %d ED %d\n", missing.first, missing.second);
		memset(uchAudioData + missing.first, 0, missing.second - missing.first + 1);
	}

	m_bIsCurrentlyParsingAudioData = true;

    Locker lock(*m_pLiveReceiverMutex);
	CAudioPacketHeader g_LiveReceiverHeader;
    size_t nNumberOfMissingBlocks = vMissingBlocks.size();
    size_t iMissingIndex = 0;

    bool bCompleteFrame = false;
    int iFrameNumber = 0, nUsedLength = 0;
    int iLeftRange, iRightRange, nFrameLeftRange, nFrameRightRange;
    int nCurrentFrameLenWithMediaHeader;
    nFrameLeftRange = nOffset;	
	int numOfMissingFrames = 0;

    while(iFrameNumber < nNumberOfAudioFrames)
    {
        bCompleteFrame = true;

        nFrameLeftRange = nUsedLength + nOffset;
        nFrameRightRange = nFrameLeftRange + pAudioFramsStartingByte[ iFrameNumber ] - 1;
        nUsedLength += pAudioFramsStartingByte[ iFrameNumber ];

        //LLG("#IV# THeKing--> Audio  left, right, iframenum  = "+ Tools::IntegertoStringConvert(nFrameLeftRange)+","+Tools::IntegertoStringConvert(nFrameRightRange)+","+Tools::IntegertoStringConvert(iFrameNumber));

        while(iMissingIndex < nNumberOfMissingBlocks &&  vMissingBlocks[ iMissingIndex ].second <= nFrameLeftRange)
            ++ iMissingIndex;

        if(iMissingIndex < nNumberOfMissingBlocks)
        {
            iLeftRange = vMissingBlocks[iMissingIndex].first;
            iRightRange = vMissingBlocks[iMissingIndex].second;

            iLeftRange =  max(nFrameLeftRange, iLeftRange);
            iRightRange = min(nFrameRightRange,iRightRange);
			if (iLeftRange <= iRightRange)
			{
				HITLER("XXP@#@#MARUF LIVE FRAME INCOMPLETE. [%03d]", (iLeftRange - nFrameLeftRange));
				if (nFrameLeftRange < vMissingBlocks[iMissingIndex].first && (iLeftRange - nFrameLeftRange) >= 10)
				{
					HITLER("XXP@#@#MARUF LIVE FRAME CHECK FOR VALID HEADER");
					m_pAudioPacketHeader->CopyHeaderToInformation(uchAudioData + nFrameLeftRange + 1);
					int validHeaderLength = m_pAudioPacketHeader->GetInformation(INF_HEADERLENGTH);
					
					HITLER("XXP@#@#MARUF LIVE FRAME CHECKED FOR VALID HEADER EXISTING DATA [%02d], VALID HEADER [%02d]", iLeftRange - nFrameLeftRange, validHeaderLength);

					if (validHeaderLength > (iLeftRange - nFrameLeftRange)) {
						HITLER("XXP@#@#MARUF LIVE HEADER INCOMPLETE");
						bCompleteFrame = false;
					}
				}
				else
				{
					HITLER("XXP@#@#MARUF LIVE INCOMLETE FOR START INDEX IN MISSING POSITION");
					bCompleteFrame = false;
				}
			}
        }

        ++iFrameNumber;


		if (m_pAudioCallSession->GetRole() == VIEWER_IN_CALL) {
			nCurrentFrameLenWithMediaHeader = nFrameRightRange - nFrameLeftRange + 1;
			uchAudioData[nFrameLeftRange + 1] = AUDIO_NONMUXED_LIVE_CALL_PACKET_TYPE;
			for (int i = 1600 - 1; i >= 0; i--) {
				uchAudioData[nFrameRightRange - (1599 - i)] = i;
			}
			m_pLiveAudioReceivedQueue->EnQueue(uchAudioData + nFrameLeftRange + 1, nCurrentFrameLenWithMediaHeader - 1);
			continue;
		}
		else {
			HITLER("XXP@#@#MARUF -> ERROR IN PACKET TYPE");
		}

        if( !bCompleteFrame )
        {	
			CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::ProcessAudioStreamVector AUDIO frame broken");

			numOfMissingFrames++;
			LOGENEW("live receiver continue PACKETNUMBER = %d\n", iPacketNumber);
            continue;
        }else{
            //LOGEF("THeKing--> #IV#    LiveReceiver::ProcessAudioStream Audio FRAME Completed -- FrameNumber = %d, CurrentFrameLenWithMediaHeadre = %d, audioFrameLength = %d ",audioFrameNumber , nFrameRightRange - nFrameLeftRange + 1, audioFrameLength);
        }

        nCurrentFrameLenWithMediaHeader = nFrameRightRange - nFrameLeftRange + 1;

        m_pLiveAudioReceivedQueue->EnQueue(uchAudioData + nFrameLeftRange +1 , nCurrentFrameLenWithMediaHeader - 1);
    }
	m_bIsCurrentlyParsingAudioData = false;
	
	LOG_AAC("#aac#b4q# TotalAudioFrames: %d, PushedAudioFrames: %d, NumOfMissingAudioFrames: %d", nNumberOfAudioFrames, (nNumberOfAudioFrames - numOfMissingFrames), numOfMissingFrames);
}

