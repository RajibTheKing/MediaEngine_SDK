
#include "HashGenerator.h"
#include "LogPrinter.h"

namespace MediaSDK
{

	CHashGenerator::CHashGenerator()
	{
		m_nSeed = DEFAULT_SEED;
	}

	CHashGenerator::~CHashGenerator()
	{

	}

	void CHashGenerator::SetSeedNumber(unsigned int nSeed)
	{
		m_nSeed = nSeed;
	}

	unsigned long long CHashGenerator::HashSeedPair(unsigned int nSeed, unsigned int nframeNumber, unsigned int nPacketNumber)
	{

		nframeNumber++;
		nPacketNumber++;
		nPacketNumber *= 0x14B; //it's a prime number

		unsigned long long result = 0;
		int indResult = 0;

		for (int i = 0; i < 21; i++)
		{
			if (nPacketNumber & (1 << i)) result |= (1LL << indResult);
			indResult++;

			if (nframeNumber & (1 << i)) result |= (1LL << indResult);
			indResult++;

			if (nSeed & (1 << i)) result |= (1LL << indResult);
			indResult++;
		}

		return result;

	}


	int CHashGenerator::GetHashedPacketSize(unsigned int nframeNumber, unsigned int nPacketNumber)
	{

		//#define MAX_PACKET_SIZE 490
		//#define MIN_PACKET_SIZE 390

		unsigned long long hashedValue = HashSeedPair(m_nSeed, nframeNumber, nPacketNumber);

		int newSize = hashedValue%MAX_PACKET_SIZE;

		CLogPrinter_LOG(CALL_PACKET_SIZE_LOG, "CHashGenerator::Packetize GetHashedPacketSize %lld newSize %d", hashedValue, newSize);

		newSize = MIN_PACKET_SIZE + hashedValue % (RANG_SIZE);

		CLogPrinter_LOG(CALL_PACKET_SIZE_LOG, "CHashGenerator::Packetize GetHashedPacketSize 222 %lld newSize %d", hashedValue, newSize);

		/*
		while(newSize<MIN_PACKET_SIZE)
		{
		newSize+=MIN_PACKET_SIZE;
		newSize%=MAX_PACKET_SIZE;
		}
		*/

		/*if(newSize<MIN_PACKET_SIZE)
		{
		newSize+=MIN_PACKET_SIZE;
		}*/
		return newSize;
	}


	int CHashGenerator::CalculateNumberOfPackets(int nFrameNumber, int iLen)
	{

		int cnt = 0;
		int sum = 0;

		while (sum < iLen)
		{
			sum += GetHashedPacketSize(nFrameNumber, cnt);
			cnt++;
		}

		return cnt;
	}


} //namespace MediaSDK

