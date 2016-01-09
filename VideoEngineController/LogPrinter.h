#ifndef _LOG_PRINTER_H_
#define _LOG_PRINTER_H_

#define _CRT_SECURE_NO_WARNINGS

//#define __PRINT_LOG__
//#define __EXACT_LOG__
#define __SPECIFIC_LOG__
#define FILE_NAME "VideoEngineTrack.log"
#define PRIORITY CLogPrinter::DEBUGS

#define IPVLongType long long

#include <string>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif 

using namespace std;

class CLogPrinter
{

public:

	enum Priority
	{
		NONE,
		DEBUGS,
		CONFIG,
		INFO,
		WARNING,
		ERRORS
	};

	CLogPrinter();

	static void Start(Priority maxPriority, const char* logFile);
	static void Stop();
	static void SetPriority(Priority maxPriority);
	static void SetExactness(bool exact);
	static std::string GetDateTime();
	static void Write(Priority priority, const std::string message);
	static void SetLoggerPath(std::string location);
    static void WriteSpecific(Priority priority, const std::string message);
    static bool SetLoggingState(bool loggingState, int logLevel);

private:

	ofstream    fileStream;
	Priority    maxPriority;
	std::string		logFile;

	static const std::string PRIORITY_NAMES[];
	static CLogPrinter instance;
    static bool isLogEnable;

};


#endif