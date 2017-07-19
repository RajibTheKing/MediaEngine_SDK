
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
		missedFrameCounter = 0;
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

			if (!bBroken)	//If the current frame is not broken.
			{
				int nCurrentFrameLen = ((int)uchVideoData[1 + iUsedLen + 13] << 8) + uchVideoData[1 + iUsedLen + 14];

				m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen + 1, nCurrentFrameLen + PACKET_HEADER_LENGTH);
			}
			else
			{

			}

			iUsedLen += frameSizes[j];
			tillIndex = endOfThisFrame + 1;
		}
	}

	void LiveReceiver::PushVideoDataVector(int offset, unsigned char* uchVideoData, int iLen, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames)
	{
		CLogPrinter_LOG(CRASH_CHECK_LOG, "LiveReceiver::PushVideoDataVector called");

		LiveReceiverLocker lock(*m_pLiveReceiverMutex);

		CLogPrinter_LOG(CRASH_CHECK_LOG, "LiveReceiver::PushVideoDataVector Locked");

		if (numberOfFrames <= 0)
			return;

		int numberOfMissingFrames = vMissingFrames.size();
		int numOfMissingFrames = 0;

		int iUsedLen = 0, nFrames = 0;
		int tillIndex = offset;

		for (int j = 0; iUsedLen < iLen; j++)
		{
			nFrames++;

			int indexOfThisFrame = tillIndex;
			int endOfThisFrame = indexOfThisFrame + frameSizes[j] - 1;
			int commonLeft, commonRight;
			bool bBroken = false;

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

			CLogPrinter_LOG(CRASH_CHECK_LOG, "LiveReceiver::PushVideoDataVector Frame %d status %d", j, bBroken);

			if (!bBroken)	
			{
				CLogPrinter_LOG(CRASH_CHECK_LOG || BROKEN_FRAME_LOG, "LiveReceiver::PushVideoDataVector CORRECT entered number %d size %d missCounter %d", j, frameSizes[j], missedFrameCounter);

				CVideoHeader videoHeader;

				CLogPrinter_LOG(CRASH_CHECK_LOG, "LiveReceiver::PushVideoDataVector iUsedLen %d", iUsedLen);

				videoHeader.setPacketHeader(uchVideoData + 1 + iUsedLen);
				int nCurrentFrameLen = videoHeader.getPacketLength();

				

				unsigned char *p = uchVideoData + iUsedLen + 1 + videoHeader.GetHeaderLength();
                
                printf("LiveReceiver: ");
                for(int i=0;i<20;i++)
                {
                    printf("%02X ", p[i]);
                }
                printf("\n");

				int nalType = p[2] == 1 ? (p[3] & 0x1f) : (p[4] & 0x1f);
                
                CLogPrinter_LOG(CRASH_CHECK_LOG, "LiveReceiver::PushVideoDataVector nCurrentFrameLen %d, nalType = %d, j=%d", nCurrentFrameLen, nalType, j);

				if (nalType == 7)
					CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::PushVideoDataVector() found frome j = " + Tools::getText(j) + " size " + Tools::getText(frameSizes[j]));
				else
					CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::PushVideoDataVector() found frame j = " + Tools::getText(j) + " size " + Tools::getText(frameSizes[j]));

				m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen + 1, nCurrentFrameLen + videoHeader.GetHeaderLength());
			}
			else
			{
				CLogPrinter_LOG(CRASH_CHECK_LOG || BROKEN_FRAME_LOG, "LiveReceiver::PushVideoDataVector BROKENNNNNNNNNN entered number %d size %d missCounter %d", j, frameSizes[j], missedFrameCounter);

				numOfMissingFrames++;
				missedFrameCounter++;;
				CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::PushVideoDataVector video frame broken j = " + Tools::getText(j) + " size " + Tools::getText(frameSizes[j]));
			}


			iUsedLen += frameSizes[j];
			tillIndex = endOfThisFrame + 1;
		}

		LOG_AAC("#aac#b4q# TotalVideoFrames: %d, PushedVideoFrames: %d, NumOfMissingVideoFrames: %d", numberOfFrames, (numberOfFrames - numOfMissingFrames), numOfMissingFrames);
	}

} //namespace MediaSDK
