
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

#include "SmartPointer.h"
#include "CommonTypes.h"
#include "Tools.h"
#include "LogPrinter.h"

#if defined(TARGET_OS_WINDOWS_PHONE) || defined (DESKTOP_C_SHARP) 
#include <windows.h>
#elif defined(TARGET_OS_IPHONE) || defined(__ANDROID__) || defined(TARGET_IPHONE_SIMULATOR)
#endif


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
		MediaLogger(std::string filePath, LogLevel logLevel);
		~MediaLogger();
		void Init();
		void Log(LogLevel loglevel, const char *format, ...);
		void WriteLogToFile();
		std::string GetFilePath();
		LogLevel GetLogLevel();
		std::string GetThreadId2();
		std::string GetDateTime();
		
	private:
		
		LogLevel m_elogLevel;
		std::string m_sFilePath;
		std::vector<std::string> m_vLogVector;
		SmartPointer<MediaLocker> m_pMediaLoggerMutex;
		static MediaLogger instance;
		std::ofstream   m_pLoggerFile;
	};
}

#endif