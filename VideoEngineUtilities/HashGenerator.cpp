
#include "HashGenerator.h"
#include "../VideoEngineController/LogPrinter.h"

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
    nPacketNumber*=0x14B; //it's a prime number
    
    unsigned long long result = 0;
    int indResult = 0;
    
    for(int i=0;i<21;i++)
    {
        if(nPacketNumber & (1<<i) ) result |= (1LL<<indResult);
        indResult++;
        
        if(nframeNumber & (1<<i) ) result |= (1LL<<indResult);
        indResult++;
        
        if(nSeed & (1<<i) ) result |= (1LL<<indResult);
        indResult++;
    }
    
    return result;
    
}


int CHashGenerator::GetHashedPacketSize(unsigned int nframeNumber, unsigned int nPacketNumber)
{
    unsigned long long hashedValue = HashSeedPair(m_nSeed, nframeNumber, nPacketNumber);
    
    int newSize = hashedValue%MAX_PACKET_SIZE;
    
    while(newSize<MIN_PACKET_SIZE)
    {
        newSize+=MIN_PACKET_SIZE;
        newSize%=MAX_PACKET_SIZE;
    }
    
    return newSize;
}
