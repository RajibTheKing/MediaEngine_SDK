
#ifndef _MEDIA_LOGGER_H_
#define _MEDIA_LOGGER_H_

#include<string>
#include<vector>
#include <time.h>
#include <fstream>   
#include <stdarg.h>
#include <iostream>
#include <sstream>
#include <thread> 
#include <stdlib.h>

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Tools.h"
#include "LogPrinter.h"

#if defined(TARGET_OS_WINDOWS_PHONE) || defined (DESKTOP_C_SHARP) 
#include <windows.h>
#elif defined(TARGET_OS_IPHONE) || defined(__ANDROID__) || defined(TARGET_IPHONE_SIMULATOR)
#endif


#if defined(__ANDROID__)
#define MEDIA_LOGGING_PATH "/sdcard/"
#elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#define MEDIA_LOGGING_PATH std::string(getenv("HOME"))+"/Documents/"
#elif defined(DESKTOP_C_SHARP)
#define MEDIA_LOGGING_PATH "C:/"
#endif

#define MEDIA_LOG_MAX_SIZE	255
#define MEDIA_LOGGING_FOLDER_NAME "medialogs/"
#define MEDIA_LOGGING_FILE_NAME "logdump.txt"
#define MEDIA_FULL_LOGGING_PATH MEDIA_LOGGING_PATH MEDIA_LOGGING_FOLDER_NAME MEDIA_LOGGING_FILE_NAME

namespace MediaSDK
{
	enum LogLevel
	{
		NONE,
		DEBUGS,
		CONFIG,
		INFO,
		WARNING,
		ERRORS
	};
	class MediaLogger
	{
	public:
		MediaLogger(LogLevel logLevel);
		~MediaLogger();
		void Init();
		void Log(LogLevel loglevel, const char *format, ...);
		void WriteLogToFile();
		std::string GetFilePath();
		LogLevel GetLogLevel();
		std::string GetThreadID();
		std::string GetDateTime();
		
	private:
		
		LogLevel m_elogLevel;
		std::string m_sFilePath;
		std::vector<std::string> m_vLogVector;
		SmartPointer<MediaLocker> m_pMediaLoggerMutex;
		std::ofstream   m_pLoggerFileStream;
		char m_sMessage[MEDIA_LOG_MAX_SIZE];
	};
}

#endif