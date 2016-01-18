#include "Tools.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif


Tools::Tools()
{
	m_fpByte = NULL;
	m_fpShort = NULL;
}

Tools::~Tools()
{
	if (m_fpByte != NULL)
		fclose(m_fpByte);
	if (m_fpShort != NULL)
		fclose(m_fpShort);
}
std::string Tools::DoubleToString(double value){
	stringstream ss;
	ss<<value;
	return ss.str();
}

std::string Tools::LongLongToString(long long value)
{
	std::string str = "";

	while (value > 0)
	{
		str += (value % 10 + '0');
		value /= 10;
	}

	std::string result = "";
	for (int index = (int)str.size() - 1; index >= 0; index--)
	{
		char b = str[index];
		result += b;
	}

	return result;
}

int Tools::StringToIntegerConvert(std::string number)
{
	int num;
	num = atoi(number.c_str());
	return num;
}

std::string Tools::IntegertoStringConvert(int number)
{
	char buf[12];

#ifdef _WIN32

	_itoa_s(number, buf, 10);

#else

	sprintf(buf, "%d", number);

#endif

	return (std::string)buf;
}


std::string Tools::LongLongtoStringConvert(long long number)
{
	char buf[22];
	int i, j, k;
	bool negative = false;

	if (number < (long long)0)
	{
		number *= (long long)-1;
		negative = true;
	}

	for (i = 0; number; i++)
	{
		buf[i] = number % 10 + '0';
		number /= 10;
	}

	if (negative)
	{
		buf[i++] = '-';
	}

	for (j = i - 1, k = 0; k < j; k++, j--)
	{
		buf[j] ^= buf[k];
		buf[k] ^= buf[j];
		buf[j] ^= buf[k];
	}

	buf[i] = '\0';

	return (std::string)buf;
}

void Tools::SOSleep(int Timeout)
{

#ifdef _WIN32 

	Sleep(Timeout);
#else

	timespec t;
	u_int32_t seconds = Timeout / 1000;
	t.tv_sec = seconds;
	t.tv_nsec = (Timeout - (seconds * 1000)) * (1000 * 1000);
	nanosleep(&t, NULL);

#endif

}


pair<int, int> Tools::GetFramePacketFromHeader(unsigned char * packet, int &iNumberOfPackets)
{
	pair<int, int> FramePacketPair = {-1, -1};

	if(!packet)
	{
		return FramePacketPair;
	}
	int iFrameNumber = GetIntFromChar(packet, 0);
	iFrameNumber &= ~(0xFF << ((sizeof(int)-1) * 8 ));
	iNumberOfPackets = GetIntFromChar(packet, 4);
	int iPackeNumber = GetIntFromChar(packet, 8);

	//CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, "CController::currentFramePacketPair: FrameNumber: "+ Tools::IntegertoStringConvert(iFrameNumber) + " PacketNo. : "+  Tools::IntegertoStringConvert(iPackeNumber)+ " NumberOfPacket : "+  Tools::IntegertoStringConvert(iNumberOfPackets));

	FramePacketPair.first = iFrameNumber;
	FramePacketPair.second = iPackeNumber;

	return FramePacketPair;
}

int Tools::GetIntFromChar(unsigned char *packetData, int index)
{
	int result = 0;
	result += (packetData[index++] & 0xFF) << 24;
	result += (packetData[index++] & 0xFF) << 16;
	result += (packetData[index++] & 0xFF) << 8;
	result += (packetData[index] & 0xFF);
	return result;
}

int Tools::GetIntFromChar(unsigned char *packetData, int index,int nLenght)
{
	int result = 0;
	int interval = 8;
	int startPoint =  interval * (nLenght-1);
	if(nLenght<=0) return -1;
	for(int i=startPoint; i >= 0 ; i-=interval)
	{
		result += (packetData[index++] & 0xFF) << i;
	}

	return result;
}

#define USE_CPP_11_TIME

#ifdef  USE_CPP_11_TIME
#include <ctime>
#include <chrono>
#endif

LongLong  Tools::CurrentTimestamp()
{

#ifndef USE_CPP_11_TIME
	struct timeval te;
	gettimeofday(&te, NULL); // get current time
	LongLong milliseconds =  te.tv_sec* + te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
	// printf("milliseconds: %lld\n", milliseconds);
	return milliseconds;
#else


	namespace sc = std::chrono;

	auto time = sc::system_clock::now(); // get the current time

	auto since_epoch = time.time_since_epoch(); // get the duration since epoch

	// I don't know what system_clock returns
	// I think it's uint64_t nanoseconds since epoch
	// Either way this duration_cast will do the right thing
	auto millis = sc::duration_cast<sc::milliseconds>(since_epoch);

	long long now = millis.count(); // just like java (new Date()).getTime();

	return now;
#endif
}

void Tools::WriteToFile(short* in_Data, int count)
{
	printf("Writing to shortFile.pcm\n");
	if (m_fpShort == NULL)
	{
		m_fpShort = fopen("shortFile.pcm", "wb");
	}
	fwrite(in_Data, 2, count, m_fpShort);
}

void Tools::WriteToFile(unsigned char* in_Data, int count)
{

	printf("Writing to bytefile.test\n");
	if (m_fpByte == NULL)
	{
		m_fpByte = fopen("byteFile.test", "wb");
	}
	fwrite(in_Data, 1, count, m_fpByte);

}