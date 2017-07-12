
#ifndef _MEDIA_LOGGER_H_
#define _MEDIA_LOGGER_H_

#include<string>
#include<vector>
#include <time.h>
#include <ctime>
#include <fstream>   
#include <stdarg.h>
#include <iostream>
#include <sstream>
#include <thread> 
#include <stdlib.h>
#include <stdio.h>

#include "SmartPointer.h"
#include "CommonTypes.h"


#define MEDIA_LOGGING_FOLDER_NAME "MediaLogs"

#if defined(__ANDROID__)
#define MEDIA_LOGGING_PATH "/sdcard/" MEDIA_LOGGING_FOLDER_NAME "/"
#define MEDIA_LOGCAT_TAG "MediaSDK"
#elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#define MEDIA_LOGGING_PATH std::string(getenv("HOME"))+"/Documents/" MEDIA_LOGGING_FOLDER_NAME "/"
#elif defined(DESKTOP_C_SHARP)
#define MEDIA_LOGGING_PATH "C:\\" MEDIA_LOGGING_FOLDER_NAME "\\"
#endif

#define MIN_BUFFERED_LOG 5
#define MAX_LOG_WRITE 100
#define THREAD_SLEEP_TIME 250
#define MEDIA_LOG_MAX_SIZE	512

#define MEDIA_LOGGING_FILE_NAME "MediaLog.log"
#define MEDIA_FULL_LOGGING_PATH MEDIA_LOGGING_PATH MEDIA_LOGGING_FILE_NAME

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
		void Release();
		void Log(LogLevel loglevel, const char *format, ...);
		std::string GetFilePath();
		LogLevel GetLogLevel();

	private:
		bool CreateLogDirectory();
		void WriteLogToFile();
		size_t GetThreadID(char* buffer);
		size_t GetDateTime(char* buffer);

		void StartMediaLoggingThread();
		void StopMediaLoggingThread();
		static void* CreateLoggingThread(void* param);
		
	private:
		
		LogLevel m_elogLevel;
		std::string m_sFilePath;
		std::vector<std::string> m_vLogVector;
		SmartPointer<CLockHandler> m_pMediaLoggerMutex;
		std::ofstream   m_pLoggerFileStream;
		char m_sMessage[MEDIA_LOG_MAX_SIZE];

		bool m_bMediaLoggingThreadRunning;
		std::thread m_threadInstance;

		/// File System Error
		bool m_bFSError; 
	};
}

#endif