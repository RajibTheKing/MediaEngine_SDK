#include "LiveAudioParserForPublisher.h"
#include "LogPrinter.h"
#include "AudioPacketHeader.h"
#include "ThreadTools.h"
#include "AudioMacros.h"

CLiveAudioParserForPublisher::CLiveAudioParserForPublisher(std::vector<LiveAudioDecodingQueue*> vAudioFarEndBufferVector){
	m_vAudioFarEndBufferVector = vAudioFarEndBufferVector;
	m_pLiveReceiverMutex.reset(new CLockHandler);
	m_bIsCurrentlyParsingAudioData = false;
	m_bIsRoleChanging = false;

	m_pAudioPacketHeader = new CAudioPacketHeader();
}

CLiveAudioParserForPublisher::~CLiveAudioParserForPublisher(){
	SHARED_PTR_DELETE(m_pLiveReceiverMutex);
	delete m_pAudioPacketHeader;
}

void CLiveAudioParserForPublisher::SetRoleChanging(bool bFlah){
	m_bIsRoleChanging = bFlah;
}

bool CLiveAudioParserForPublisher::GetRoleChanging(){
	return m_bIsRoleChanging;
}

bool CLiveAudioParserForPublisher::IsParsingAudioData(){
	return m_bIsCurrentlyParsingAudioData;
}

void CLiveAudioParserForPublisher::ProcessLiveAudio(int iId, int nOffset, unsigned char* uchAudioData, int nDataLength, int *pAudioFramsStartingByte, int nNumberOfAudioFrames, std::vector< std::pair<int, int> > vMissingBlocks){
	if (m_bIsRoleChanging)
	{
		return;
	}

	for (auto &missing : vMissingBlocks)
	{
		HITLER("XXP@#@#MARUF LIVE ST %d ED %d", missing.first, missing.second);
		int left = max(nOffset, missing.first);
		if (left < missing.second)
		{
			memset(uchAudioData + left, 0, missing.second - left + 1);
		}
	}

	m_bIsCurrentlyParsingAudioData = true;

	Locker lock(*m_pLiveReceiverMutex);
	CAudioPacketHeader g_LiveReceiverHeader;
	size_t nNumberOfMissingBlocks = vMissingBlocks.size();
	size_t iMissingIndex = 0;

	bool bCompleteFrame = false;
	bool bCompleteFrameHeader = false;
	int iFrameNumber = 0, nUsedLength = 0;
	int iLeftRange, iRightRange, nFrameLeftRange, nFrameRightRange;
	int nCurrentFrameLenWithMediaHeader;
	nFrameLeftRange = nOffset;
	int numOfMissingFrames = 0;
	int nProcessedFramsCounter = 0;

	while (iFrameNumber < nNumberOfAudioFrames)
	{
		bCompleteFrame = true;
		bCompleteFrameHeader = true;

		nFrameLeftRange = nUsedLength + nOffset;
		nFrameRightRange = nFrameLeftRange + pAudioFramsStartingByte[iFrameNumber] - 1;
		nUsedLength += pAudioFramsStartingByte[iFrameNumber];

		while (iMissingIndex < nNumberOfMissingBlocks && vMissingBlocks[iMissingIndex].second <= nFrameLeftRange)
			++iMissingIndex;

		if (iMissingIndex < nNumberOfMissingBlocks)
		{
			iLeftRange = vMissingBlocks[iMissingIndex].first;
			iRightRange = vMissingBlocks[iMissingIndex].second;

			iLeftRange = max(nFrameLeftRange, iLeftRange);
			iRightRange = min(nFrameRightRange, iRightRange);

			if (iLeftRange <= iRightRange)	//The frame is not complete.
			{
				bCompleteFrame = false;
				HITLER("XXP@#@#MARUF LIVE FRAME INCOMPLETE. [%03d]", (iLeftRange - nFrameLeftRange));
				if (nFrameLeftRange < vMissingBlocks[iMissingIndex].first && (iLeftRange - nFrameLeftRange) >= MINIMUM_AUDIO_HEADER_SIZE)
				{
					HITLER("XXP@#@#MARUF LIVE FRAME CHECK FOR VALID HEADER");
					m_pAudioPacketHeader->CopyHeaderToInformation(uchAudioData + nFrameLeftRange + 1);
					int validHeaderLength = m_pAudioPacketHeader->GetInformation(INF_HEADERLENGTH);

					HITLER("XXP@#@#MARUF LIVE FRAME CHECKED FOR VALID HEADER EXISTING DATA [%02d], VALID HEADER [%02d]", iLeftRange - nFrameLeftRange, validHeaderLength);

					if (validHeaderLength >(iLeftRange - nFrameLeftRange)) {
						HITLER("XXP@#@#MARUF LIVE HEADER INCOMPLETE");
						bCompleteFrameHeader = false;
					}
				}
				else
				{
					HITLER("XXP@#@#MARUF LIVE INCOMLETE FOR START INDEX IN MISSING POSITION");
					bCompleteFrameHeader = false;
				}
			}
		}

		++iFrameNumber;
		HITLER("#@#@ livereceiver receivedpacket frameno:%d", iFrameNumber);

		if (!bCompleteFrame)
		{
			CLogPrinter_WriteFileLog(CLogPrinter::INFO, WRITE_TO_LOG_FILE, "LiveReceiver::ProcessAudioStreamVector AUDIO frame broken");

			numOfMissingFrames++;
			HITLER("XXP@#@#MARUF -> live receiver continue PACKETNUMBER = %d", iFrameNumber);
			continue;
		}

		nCurrentFrameLenWithMediaHeader = nFrameRightRange - nFrameLeftRange + 1;
		nProcessedFramsCounter++;
		if (m_vAudioFarEndBufferVector[iId])
		{
			m_vAudioFarEndBufferVector[iId]->EnQueue(uchAudioData + nFrameLeftRange + 1, nCurrentFrameLenWithMediaHeader - 1);
		}
	}

	HITLER("#@LR -> Totla= %d Used = %d Missing = %d", nNumberOfAudioFrames, nProcessedFramsCounter, nNumberOfAudioFrames - nProcessedFramsCounter);
	m_bIsCurrentlyParsingAudioData = false;

	LOG_AAC("#aac#b4q# TotalAudioFrames: %d, PushedAudioFrames: %d, NumOfMissingAudioFrames: %d", nNumberOfAudioFrames, (nNumberOfAudioFrames - numOfMissingFrames), numOfMissingFrames);
}