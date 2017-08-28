
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

	bool LiveReceiver::isComplement(int firstFrame, int secondFrame, int offset, int numberOfFrames, int *frameSizes, std::vector<std::pair<int, int>>& vMissingFrames)
	{
		if (firstFrame < 0 || firstFrame >= numberOfFrames || secondFrame < 0 || secondFrame >= numberOfFrames)
		{
			return false;
		}
		if (frameSizes[firstFrame] != frameSizes[secondFrame])
		{
			return false;
		}
		
		int firstFrameStartPos, secondFrameStartPos;
		
		for (int i = 0, current = offset; i < numberOfFrames; i++, current += frameSizes[i])
		{
			if (i == firstFrame)
			{
				firstFrameStartPos = current;
			}
			else if (i == secondFrame)
			{
				secondFrameStartPos = current;
			}
			else if (i > firstFrame && i > secondFrame)
			{
				break;
			}
		}

		if (firstFrameStartPos > secondFrameStartPos)
		{
			swap(firstFrameStartPos, secondFrameStartPos);
		}

		bool flag = true;
		int numberOfMissingFrames = vMissingFrames.size();

		for (int i = 0; i < frameSizes[firstFrame]; i++)
		{
			bool missingFromFirstFrame = false, missingFromSecondFrame = false;
			for (int j = 0; j < numberOfFrames; j++)
			{
				if (firstFrameStartPos + i >= vMissingFrames[j].first && firstFrameStartPos + i <= vMissingFrames[i].second)
				{
					missingFromFirstFrame = true;
				}
				if (secondFrameStartPos + i >= vMissingFrames[j].first && secondFrameStartPos + i <= vMissingFrames[i].second)
				{
					missingFromSecondFrame = true;
				}
				if (missingFromFirstFrame == true && missingFromSecondFrame == true)
				{
					flag = false;
					j = numberOfFrames;
					i = frameSizes[firstFrame];
				}
			}
		}

		return flag;
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
		int tillIndex = offset + 3;

		if (numberOfMissingFrames)
		{
			int rightMost = vMissingFrames[numberOfMissingFrames - 1].second;

			if (rightMost > iLen + 100)
				CLogPrinter_LOG(BROKEN_FRAME_LOG, "LiveReceiver::PushVideoDataVector numberOfMissingFrames %d left %d right %d videoLength %d AUDIO", numberOfMissingFrames, vMissingFrames[0].first, vMissingFrames[0].second, iLen);
			else
				CLogPrinter_LOG(BROKEN_FRAME_LOG, "LiveReceiver::PushVideoDataVector numberOfMissingFrames %d left %d right %d videoLength %d", numberOfMissingFrames, vMissingFrames[0].first, vMissingFrames[0].second, iLen);
		}

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

				videoHeader.SetPacketHeader(uchVideoData + 1 + iUsedLen);
				int nCurrentFrameLen = videoHeader.GetPacketLength();

				

				unsigned char *p = uchVideoData + iUsedLen + 1 + videoHeader.GetHeaderLength();
                
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
