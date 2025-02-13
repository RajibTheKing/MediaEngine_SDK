
#ifndef IPV_HASH_GENERATOR_H
#define IPV_HASH_GENERATOR_H

#include <string>

#include "SmartPointer.h"
#include "CommonTypes.h"


#define MAX_PACKET_SIZE 920
#define MIN_PACKET_SIZE 820
#define RANG_SIZE MAX_PACKET_SIZE - MIN_PACKET_SIZE

#define DEFAULT_SEED 23232323
#define SEED_MOD_VALUE 100000000

namespace MediaSDK
{

	class CHashGenerator
	{
	public:
		CHashGenerator();
		~CHashGenerator();


		void SetSeedNumber(unsigned int nSeed);
		int GetHashedPacketSize(unsigned int nframeNumber, unsigned int nPacketNumber);
		unsigned long long HashSeedPair(unsigned int nSeed, unsigned int nframeNumber, unsigned int nPacketNumber);
		int CalculateNumberOfPackets(int nFrameNumber, int iLen);

	private:
		unsigned int m_nSeed;
		SharedPointer<CLockHandler> m_pHashGeneratorMutex;
	};

} //namespace MediaSDK

#endif  //End of HASH_GENERATOR_H
