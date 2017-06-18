
#include "LiveReceiver.h"
#include "VideoHeader.h"
#include "Tools.h"
#include "ThreadTools.h"
#include "CommonElementsBucket.h"
#include "LiveVideoDecodingQueue.h"

namespace MediaSDK
{

	LiveReceiver::LiveReceiver(CCommonElementsBucket* sharedObject) :
		m_pCommonElementsBucket(sharedObject)

	{
		m_pLiveVideoDecodingQueue = NULL;
		m_pLiveReceiverMutex.reset(new CLockHandler);
	}

	LiveReceiver::~LiveReceiver()
	{
		SHARED_PTR_DELETE(m_pLiveReceiverMutex);
	}

	void LiveReceiver::SetVideoDecodingQueue(LiveVideoDecodingQueue *pQueue)
	{
		m_pLiveVideoDecodingQueue = pQueue;
	}

	void LiveReceiver::PushVideoData(unsigned char* uchVideoData, int iLen, int numberOfFrames, int *frameSizes, int numberOfMissingFrames, int *missingFrames)
	{
		LiveReceiverLocker lock(*m_pLiveReceiverMutex);

		int iUsedLen = 0, nFrames = 0;
		int packetSizeOfNetwork = m_pCommonElementsBucket->GetPacketSizeOfNetwork();
		int offset = packetSizeOfNetwork * NUMBER_OF_HEADER_FOR_STREAMING;
		int tillIndex = offset;
		//int frameCounter = 0;

		if (packetSizeOfNetwork < 0)
			return;

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
				commonLeft = max(indexOfThisFrame, missingFrames[i] * packetSizeOfNetwork);
				commonRight = min(endOfThisFrame, ((missingFrames[i] + 1) * packetSizeOfNetwork) - 1);

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

	void LiveReceiver::PushVideoDataVector(int offset, unsigned char* uchVideoData, int iLen, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
	{
		LiveReceiverLocker lock(*m_pLiveReceiverMutex);

		if (numberOfFrames <= 0)
			return;

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
				CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::PushVideoDataVector video frome broken j = " + Tools::getText(j) + " size " + Tools::getText(frameSizes[j]));

				return;
			}

			if (!bBroken)	//If the current frame is not broken.
			{
				//int nCurrentFrameLen = ((int)uchVideoData[1 + iUsedLen + 13] << 8) + uchVideoData[1 + iUsedLen + 14];

				CVideoHeader videoHeader;
				videoHeader.setPacketHeader(uchVideoData + 1 + iUsedLen);
				int nCurrentFrameLen = videoHeader.getPacketLength();

				unsigned char *p = uchVideoData + iUsedLen + 1 + videoHeader.GetHeaderLength();

				int nalType = p[2] == 1 ? (p[3] & 0x1f) : (p[4] & 0x1f);

				if (nalType == 7)
					CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::PushVideoDataVector() found frome j = " + Tools::getText(j) + " size " + Tools::getText(frameSizes[j]));
				else
					CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::PushVideoDataVector() found frame j = " + Tools::getText(j) + " size " + Tools::getText(frameSizes[j]));

				//m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen + 1, nCurrentFrameLen + PACKET_HEADER_LENGTH);
				m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen + 1, nCurrentFrameLen + videoHeader.GetHeaderLength());
			}
			else
			{
				numOfMissingFrames++;
				CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::PushVideoDataVector video frame broken j = " + Tools::getText(j) + " size " + Tools::getText(frameSizes[j]));
				//LOGEF("THeKing--> receive #####  [%d] Broken## UsedLen: %d iLen = %d\n",j, iUsedLen, iLen);
			}


			iUsedLen += frameSizes[j];
			tillIndex = endOfThisFrame + 1;
		}

		LOG_AAC("#aac#b4q# TotalVideoFrames: %d, PushedVideoFrames: %d, NumOfMissingVideoFrames: %d", numberOfFrames, (numberOfFrames - numOfMissingFrames), numOfMissingFrames);
		//    m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen, iLen + PACKET_HEADER_LENGTH);
	}

} //namespace MediaSDK
