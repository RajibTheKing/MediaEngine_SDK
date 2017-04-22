#include "AudioDePacketizer.h"
#include "AudioPacketHeader.h"
#include "LogPrinter.h"
#include "AudioCallSession.h"

AudioDePacketizer::AudioDePacketizer(CAudioCallSession * pAudioCallSession):
m_pAudioCallSession(pAudioCallSession),
m_iBlockOkayFlag(0), 
m_iPreviousPacketNumber(-1)
{
	m_pAudioPacketHeader = new CAudioPacketHeader();
	m_iAudioHeaderLength = m_pAudioPacketHeader->GetHeaderSize();
	m_nFrameLength = 0;
}

AudioDePacketizer::~AudioDePacketizer()
{
	delete m_pAudioPacketHeader;
	m_iBlockOkayFlag = 0;
}

bool AudioDePacketizer::dePacketize(unsigned char* uchBlock, int iBlockNo, int iTotalBlock, int iBlockLength, int iBlockOffset, int iPacketNumber, int nPacketLength, long long &llNow, long long &llLastTime)
 {
	HITLER("XXP@#@#MARUF BOKKOR IS RUNNING PACKET BN = %d  TB = %d  BL = %d BO = %d PN = %d PL = %d", iBlockNo, iTotalBlock, iBlockLength, iBlockOffset, iPacketNumber, nPacketLength);

	if (m_iPreviousPacketNumber == -1) {
		m_iPreviousPacketNumber = iPacketNumber;
		m_iBlockOkayFlag |= (1 << iBlockNo);
		memcpy(m_uchAudioStorageBuffer + iBlockOffset, uchBlock, iBlockLength);
		m_nFrameLength = nPacketLength;
		if ((1 << iTotalBlock) == (m_iBlockOkayFlag + 1)) 
		{ 
			HITLER("XXP@#@#MARUF Packet Return True 1");
			return true; 
		} 
	}
	else {
		if (m_iPreviousPacketNumber == iPacketNumber) {

			int nIsBlockAlreadyExists = (m_iBlockOkayFlag & (1 << iBlockNo) );

			if( nIsBlockAlreadyExists > 0)
			{
				return false;
			}

			m_iBlockOkayFlag |= (1 << iBlockNo);
			memcpy(m_uchAudioStorageBuffer + iBlockOffset, uchBlock, iBlockLength);
			m_nFrameLength = nPacketLength;
			if ((1 << iTotalBlock) == (m_iBlockOkayFlag + 1))
			{
				HITLER("XXP@#@#MARUF Packet Return True 2");
				return true;
			}
		}
		else if(m_iPreviousPacketNumber < iPacketNumber){			
			m_iBlockOkayFlag = 0;
			memset(m_uchAudioStorageBuffer, 0, nPacketLength);
			m_iPreviousPacketNumber = iPacketNumber;
			m_iBlockOkayFlag |= (1 << iBlockNo);
			memcpy(m_uchAudioStorageBuffer + iBlockOffset, uchBlock, iBlockLength);
			m_nFrameLength = nPacketLength;
			if ((1 << iTotalBlock) == (m_iBlockOkayFlag + 1))
			{
				HITLER("XXP@#@#MARUF Packet Return True 2");
				return true;
			}
		}
		else {
			HITLER("XXP@#@#MARUF PACKET REMOVED DUE TO LOW PACKET NUMBER. PREV [%d] CUR [%d]", m_iPreviousPacketNumber, iPacketNumber);
			return false;
		}
	}
	return false;
}

int AudioDePacketizer::GetCompleteFrame(unsigned char* uchFrame){
	memcpy(uchFrame, m_uchAudioStorageBuffer, m_nFrameLength);
	m_iBlockOkayFlag = 0;
	m_iPreviousPacketNumber++;
	memset(m_uchAudioStorageBuffer, 0, sizeof m_uchAudioStorageBuffer);
	return m_nFrameLength;
}

void AudioDePacketizer::SentIncompleteFrame(int iLastPacketNumber, long long &llNow, long long &llLastTime)
{
	HITLER("XXP@#@#MARUF SENDING INCOMPLETE FRAME");
	memcpy(m_saDataToPlay, m_uchAudioStorageBuffer, m_nFrameLength);
	int nFrameLenInShort = m_nFrameLength / 2;

	m_pAudioCallSession->DumpDecodedFrame(m_saDataToPlay, nFrameLenInShort);

	long long llTemp1 = -1, llTemp2 = -1;
	m_pAudioCallSession->SendToPlayer(m_saDataToPlay, nFrameLenInShort, llNow, llLastTime, iLastPacketNumber);
}

void AudioDePacketizer::ResetDepacketizer()
{
	m_iPreviousPacketNumber = -1;
	memset(m_uchAudioStorageBuffer, 0, sizeof m_uchAudioStorageBuffer);
}