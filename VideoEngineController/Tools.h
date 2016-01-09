#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <stdio.h>
#include <string>
#include <cstdlib>

#include "AudioVideoEngineDefinitions.h"
#include <utility>
#include <queue>
#include <deque>
#include <sstream>

class Tools;



using namespace std;

class Tools
{
public:

	void SOSleep(int Timeout);
	LongLong CurrentTimestamp();
	std::string DoubleToString(double value);
	static int StringToIntegerConvert(std::string number);
	static std::string IntegertoStringConvert(int number);
	static std::string LongLongtoStringConvert(LongLong number);
	static std::string LongLongToString(LongLong value);


	static pair<int, int> GetFramePacketFromHeader(unsigned char * packet, int &iNumberOfPackets);
	static int GetIntFromChar(unsigned char *packetData, int index);
	static int GetIntFromChar(unsigned char *packetData, int index,int nLenght);
};
#define funcline(...) CLogPrinter::WriteSpecific(CLogPrinter::DEBUGS, std::string(__func__) + " : " + Tools::IntegertoStringConvert(__LINE__) + ":" + std::string(__VA_ARGS__));
#endif