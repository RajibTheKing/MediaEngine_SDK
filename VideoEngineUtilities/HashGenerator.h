
#ifndef HASH_GENERATOR_H
#define HASH_GENERATOR_H
#include <string>

#include "AudioVideoEngineDefinitions.h"
#include "SmartPointer.h"
#include "LockHandler.h"


#define MAX_PACKET_SIZE 490
#define MIN_PACKET_SIZE 390
#define RANG_SIZE MAX_PACKET_SIZE - MIN_PACKET_SIZE

#define DEFAULT_SEED 23232323

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
	SmartPointer<CLockHandler> m_pHashGeneratorMutex;
};

#endif  //End of HASH_GENERATOR_H
