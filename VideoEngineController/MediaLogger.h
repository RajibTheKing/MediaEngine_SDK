
#ifndef _MEDIA_LOGGER_H_
#define _MEDIA_LOGGER_H_

#include<string>
#include<vector>
#include <time.h>
#include "SmartPointer.h"
#include "CommonTypes.h"

#if defined(TARGET_OS_WINDOWS_PHONE) || defined (DESKTOP_C_SHARP) 
#include <windows.h>
#include <stdarg.h>
#elif defined(TARGET_OS_IPHONE) || defined(__ANDROID__) || defined(TARGET_IPHONE_SIMULATOR)
#include <sys/prctl.h>
#include <pthread.h>
#endif

#define va_start _crt_va_start
#define va_arg _crt_va_arg
#define va_end _crt_va_end
void _CRTIMP __cdecl _vacopy(_Out_ va_list *, _In_ va_list);
#define va_copy(apd, aps) _vacopy(&(apd), aps)

namespace MediaSDK
{
	class MediaLogger
	{
	private:
		enum LogLevel
		{
			NONE,
			DEBUGS,
			CONFIG,
			INFO,
			WARNING,
			ERRORS
		};
		LogLevel m_elogLevel;
		std::string m_sFilePath;
		FILE *m_pLoggerFile;
		std::vector<std::string> m_vLogVector;
		SmartPointer<MediaLocker> m_pMediaLoggerMutex;

	public : 
		MediaLogger(std::string filePath, LogLevel logLevel);
		~MediaLogger();
		void Init();
		inline void Log(LogLevel loglevel, const char *format, ...);
		void WriteLogToFile();
		inline std::string GetFilePath();
		inline LogLevel GetLogLevel();
		std::string GetDateTime();
		std::string GetThreadId();
	};
}

#endif