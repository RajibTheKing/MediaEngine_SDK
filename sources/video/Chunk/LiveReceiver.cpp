#include "LiveReceiver.h"
#include "VideoHeader.h"
#include "Tools.h"
#include "ThreadTools.h"
#include "CommonElementsBucket.h"
#include "LiveVideoDecodingQueue.h"

namespace MediaSDK
{

	LiveReceiver::LiveReceiver(CCommonElementsBucket* sharedObject) :
		m_pCommonElementsBucket(sharedObject),
		m_nGotFrameCounter(0),
		m_nMissedFrameCounter(0),
		m_nIFrameGap(0),
		m_nPreviousIFrameNumber(0)

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

	bool LiveReceiver::isComplement(int firstFrame, int secondFrame, int offset, unsigned char* uchVideoData, int numberOfFrames, int *frameSizes, std::vector<std::pair<int, int>>& vMissingFrames, unsigned char* constructedFrame)
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

		bool flag = true;

		int numberOfMissingFrames = vMissingFrames.size();

		for (int i = 0; i < frameSizes[firstFrame]; i++)
		{
			bool missingFromFirstFrame = false, missingFromSecondFrame = false;

			for (int j = 0; j < numberOfMissingFrames; j++)
			{
				if (firstFrameStartPos + i >= vMissingFrames[j].first && firstFrameStartPos + i <= vMissingFrames[j].second)
				{
					missingFromFirstFrame = true;
				}
				if (secondFrameStartPos + i >= vMissingFrames[j].first && secondFrameStartPos + i <= vMissingFrames[j].second)
				{
					missingFromSecondFrame = true;
				}
				if (missingFromFirstFrame == true && missingFromSecondFrame == true)
				{
					flag = false;

					j = numberOfMissingFrames;
					i = frameSizes[firstFrame];
				}
			}

			if (missingFromSecondFrame == false)
			{
				constructedFrame[i] = uchVideoData[secondFrameStartPos + i];
			}
			else if (missingFromFirstFrame == false)
			{
				constructedFrame[i] = uchVideoData[firstFrameStartPos + i];
			}
		}

		return flag;
	}

	bool LiveReceiver::isComplementEfficient(int firstFrame, int secondFrame, int offset, unsigned char* uchVideoData, int numberOfFrames, int *frameSizes, std::vector<std::pair<int, int>>& vMissingFrames, unsigned char* constructedFrame)
	{
		if (firstFrame < 0 || firstFrame >= numberOfFrames || secondFrame < 0 || secondFrame >= numberOfFrames)
		{
			return false;
		}
		if (numberOfFrames < 2)
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

		int firstFrameEndPos = firstFrameStartPos + frameSizes[firstFrame] - 1, secondFrameEndPos = secondFrameStartPos + frameSizes[secondFrame] - 1;
		int commonLeft, commonRight;

		std::sort(vMissingFrames.begin(), vMissingFrames.end());

		vector<pair<int, int>> firstFrameMissingParts, secondFrameMissingParts;

		for (int i = 0; i < vMissingFrames.size(); i++)
		{
			commonLeft = max(vMissingFrames[i].first, firstFrameStartPos);
			commonRight = min(vMissingFrames[i].second, firstFrameEndPos);
			if (commonLeft <= commonRight)
			{
				firstFrameMissingParts.push_back({ commonLeft - firstFrameStartPos, commonRight - firstFrameStartPos });
			}
			commonLeft = max(vMissingFrames[i].first, secondFrameStartPos);
			commonRight = min(vMissingFrames[i].second, secondFrameEndPos);
			if (commonLeft <= commonRight)
			{
				secondFrameMissingParts.push_back({ commonLeft - secondFrameStartPos, commonRight - secondFrameStartPos });
			}
		}

		firstFrameMissingParts.push_back({ frameSizes[firstFrame], frameSizes[firstFrame] });
		secondFrameMissingParts.push_back({ frameSizes[secondFrame], frameSizes[secondFrame] });

		bool flag = true;
		int numberOfMissingPartsFirstFrame = firstFrameMissingParts.size(), numberOfMissingPartsSecondFrame = secondFrameMissingParts.size();

		int startNow = 0;
		for (int i = 0, j = 0; startNow < frameSizes[firstFrame];)
		{
			bool success = false;

			if (i < numberOfMissingPartsFirstFrame)
			{
				while (i < numberOfMissingPartsFirstFrame && firstFrameMissingParts[i].second < startNow) i++;
				if (i < numberOfMissingPartsFirstFrame && firstFrameMissingParts[i].first > startNow)
				{
					success = true;
					memcpy(constructedFrame + startNow - offset, uchVideoData + firstFrameStartPos + startNow, firstFrameMissingParts[i].first - startNow);
					startNow = firstFrameMissingParts[i].first;
				}
			}

			if (j < numberOfMissingPartsSecondFrame)
			{
				while (j < numberOfMissingPartsSecondFrame && secondFrameMissingParts[j].second < startNow) j++;
				if (j < numberOfMissingPartsSecondFrame && secondFrameMissingParts[j].first > startNow)
				{
					success = true;
					memcpy(constructedFrame + startNow - offset, uchVideoData + secondFrameStartPos + startNow, secondFrameMissingParts[j].first - startNow);
					startNow = secondFrameMissingParts[j].first;
				}
			}

			if (success == false)
			{
				flag = false;
				break;
			}
		}

		return flag;
	}

	void LiveReceiver::PushVideoDataVector(int offset, unsigned char* uchVideoData, int iLen, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames, int serviceType)
	{
		CLogPrinter_LOG(CRASH_CHECK_LOG, "LiveReceiver::PushVideoDataVector called");

		LiveReceiverLocker lock(*m_pLiveReceiverMutex);

		CLogPrinter_LOG(CRASH_CHECK_LOG, "LiveReceiver::PushVideoDataVector Locked");

		if (numberOfFrames <= 0)
			return;

		int numberOfMissingFrames = vMissingFrames.size();
		int numOfMissingFrames = 0;

		int iUsedLen = 3, nFrames = 0;
		int tillIndex = offset + 3;

		if ((int)uchVideoData[0] != 0 && serviceType != SERVICE_TYPE_CHANNEL)
		{
			CLogPrinter_LOG(WRONG_ENTRY_LOG, "LiveReceiver::PushVideoDataVector Wrong Entry in concealing");

			bool success = false;
			
			if ((int)uchVideoData[0] == 2 )
				success = isComplement((int)uchVideoData[1], (int)uchVideoData[2], 3, uchVideoData, numberOfFrames, frameSizes, vMissingFrames, m_pBackupData);

			if (success)
			{
				CVideoHeader videoHeader;

				CLogPrinter_LOG(CRASH_CHECK_LOG, "LiveReceiver::PushVideoDataVector iUsedLen %d", iUsedLen);

				videoHeader.SetPacketHeader(uchVideoData + 1 + iUsedLen);
				int nCurrentFrameLen = videoHeader.GetPacketLength();

				m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen + 1, nCurrentFrameLen + videoHeader.GetHeaderLength());
			}
		}
		
		if (serviceType == SERVICE_TYPE_CHANNEL)
		{
			iUsedLen = 0;
			tillIndex = offset;
		}

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

			if (serviceType != SERVICE_TYPE_CHANNEL && (int)uchVideoData[0] != 0 && (j == (int)uchVideoData[1] || j == (int)uchVideoData[2]))
			{
				iUsedLen += frameSizes[j];
				continue;
			}

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
				CLogPrinter_LOG(CRASH_CHECK_LOG || BROKEN_FRAME_LOG, "LiveReceiver::PushVideoDataVector CORRECT entered number %d size %d missCounter %d", j, frameSizes[j], m_nMissedFrameCounter);

				CVideoHeader videoHeader;

				CLogPrinter_LOG(CRASH_CHECK_LOG, "LiveReceiver::PushVideoDataVector iUsedLen %d", iUsedLen);

				videoHeader.SetPacketHeader(uchVideoData + 1 + iUsedLen);
				int nCurrentFrameLen = videoHeader.GetPacketLength();

				

				unsigned char *p = uchVideoData + iUsedLen + 1 + videoHeader.GetHeaderLength();
                
				int nalType = p[2] == 1 ? (p[3] & 0x1f) : (p[4] & 0x1f);
                
                CLogPrinter_LOG(CRASH_CHECK_LOG, "LiveReceiver::PushVideoDataVector nCurrentFrameLen %d, nalType = %d, j=%d", nCurrentFrameLen, nalType, j);

				if (nalType == 7)
				{
					m_nIFrameGap = m_nGotFrameCounter - m_nPreviousIFrameNumber;
					m_nPreviousIFrameNumber = m_nGotFrameCounter;

					CLogPrinter_LOG(GOT_FRAME_TYPE_LOG, "LiveReceiver::PushVideoDataVector got correct IIIIIIIIIIIIIII Frame Gap %d", m_nIFrameGap);
				}
				else
					CLogPrinter_LOG(GOT_FRAME_TYPE_LOG, "LiveReceiver::PushVideoDataVector got correct P frame");

				m_pLiveVideoDecodingQueue->Queue(uchVideoData + iUsedLen + 1, nCurrentFrameLen + videoHeader.GetHeaderLength());
			}
			else
			{
				CLogPrinter_LOG(CRASH_CHECK_LOG || BROKEN_FRAME_LOG, "LiveReceiver::PushVideoDataVector BROKENNNNNNNNNN entered number %d size %d missCounter %d", j, frameSizes[j], m_nMissedFrameCounter);

				numOfMissingFrames++;
				m_nMissedFrameCounter++;;
				CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::PushVideoDataVector video frame broken j = " + Tools::getText(j) + " size " + Tools::getText(frameSizes[j]));
			}


			iUsedLen += frameSizes[j];
			tillIndex = endOfThisFrame + 1;

			m_nGotFrameCounter++;
		}

		LOG_AAC("#aac#b4q# TotalVideoFrames: %d, PushedVideoFrames: %d, NumOfMissingVideoFrames: %d", numberOfFrames, (numberOfFrames - numOfMissingFrames), numOfMissingFrames);
	}

} //namespace MediaSDK
