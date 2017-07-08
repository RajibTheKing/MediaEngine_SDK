
#ifndef IPV_LOG_PRINTER_H
#define IPV_LOG_PRINTER_H

#include "CommonTypes.h"

//#define _CRT_SECURE_NO_WARNINGS


//#define __PRINT_LOG__
//#define __EXACT_LOG__
//#define __SPECIFIC_LOG__
//#define __SPECIFIC_LOG4__
//#define __SPECIFIC_LOG5__
//#define __SPECIFIC_LOG3__
//#define __INSTENT_TEST_LOG__
//#define __OPERATION_TIME_LOG__
//#define __QUEUE_TIME_LOG__
//#define __PACKET_LOSS_INFO_LOG__
//#define __THREAD_LOG__
//#define __BITRATE_CHNANGE_LOG__

//#define  __SPECIFIC_LOG6__

#define LLG(a)     CLogPrinter_WriteSpecific6(CLogPrinter::INFO,a);

#define ON 1
#define OFF 0

#define LOG_ENABLED

#define WRITE_TO_LOG_FILE		OFF

#define PUBLISHER_TIME_LOG		OFF
#define CRASH_CHECK_LOG			OFF
#define API_FLOW_CHECK_LOG		OFF
#define PACKET_DETAILS_LOG		OFF
#define INSTENT_TEST_LOG_2		OFF
#define INSTENT_TEST_LOG		OFF
#define CHECK_CAPABILITY_LOG	OFF
#define QUEUE_OVERFLOW_LOG		OFF
#define OPERATION_TIME_LOG		OFF
#define QUEUE_TIME_LOG			OFF
#define PACKET_LOSS_INFO_LOG	OFF
#define THREAD_LOG				OFF
#define BITRATE_CHNANGE_LOG		OFF
#define DEPACKETIZATION_LOG		OFF
#define VIDEO_NOTIFICATION_LOG  OFF





#if  defined(__ANDROID__) && defined(LOG_ENABLED)

#include <android/log.h>

#define LOG_TAG "LibraryLog"

#define LOGE_MAIN(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define LOGF(...) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGE(...) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGEF(...) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define __LOG(...) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define PRT(...) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOG_AAC(...)  //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGENEW(...) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define HITLER(...)	 //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOG_50MS(...) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGT(...) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGSS(...) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOG18(...) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define HITLERSS(...) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define MR_DEBUG(...) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define DOG(...) //__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define MANSUR(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else

#define LOG_AAC(...)  
#define LOGF(...) 
#define LOGE(...)
#define LOGEF(...)
#define __LOG(...)
#define PRT(...)
#define LOGENEW(...)
#define HITLER(...)
#define LOG_50MS(...)
#define LOGT(...)
#define LOGSS(...)
#define LOG18(...) 
#define HITLERSS(...)
#define MR_DEBUG(...)
#define LOGE_MAIN(...)
#define DOG(...)
#define MANSUR(...)
#endif




#ifdef __ANDROID__
#define FILE_NAME "/sdcard/VideoEngineTrack.txt"
#else
#define FILE_NAME "VideoEngineTrack.log"
#endif

#define PRIORITY CLogPrinter::DEBUGS


#include "../VideoEngineUtilities/SmartPointer.h"

#include <stdio.h>

#include <string>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <stdarg.h>


#else
#include <sys/time.h>
#endif 

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#ifdef LOG_ENABLED
	//Do Nothing
#else
#define printf(...)
#endif
#endif

#define LOGS(a)     CLogPrinter_WriteSpecific6(CLogPrinter::INFO,a);

namespace MediaSDK
{

#if defined(DESKTOP_C_SHARP) || defined(TARGET_OS_WINDOWS_PHONE)
static FILE *logfp = NULL;
#define printg(X,...) _RPT1(0,X,__VA_ARGS__)

#ifdef LOG_ENABLED
#define printf(...) printg(__VA_ARGS__,"")
#else
#define printf(...)
#endif

#define printk(...) printg(__VA_ARGS__,"")
//#define printf(...)
#define printFile(...) if(!logfp) {logfp = fopen("log.txt", "wb");} fprintf(logfp, __VA_ARGS__);
#define printfiledone() fclose(logfp);
#endif

#ifdef TARGET_OS_WINDOWS_PHONE
	typedef __int64 IPVLongType;
    #else
	typedef long long IPVLongType;
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
	~CLogPrinter();

	static void Start(Priority maxPriority, const char* logFile);
	static void Stop();
	static void SetPriority(Priority maxPriority);
	static void SetExactness(bool exact);
	static std::string GetDateTime();
    
    static void Log(const char *format, ...);
	static void LogWithCheck(int isLogEnabled, const char *format, ...);
    static void Argument_to_String(string &dst, const char *format, va_list ap);
    
	static void Write(Priority priority, const std::string message);
	static void SetLoggerPath(std::string location);
    static void WriteSpecific(Priority priority, const std::string message);
    static void WriteSpecific2(Priority priority, const std::string message);
    static bool SetLoggingState(bool loggingState, int logLevel);
	static long long WriteForOperationTime(Priority priority, const std::string message, long long prevTime = 0);
	static void WriteForQueueTime(Priority priority, const std::string message);
	static void WriteForPacketLossInfo(Priority priority, const std::string message);

	static long long WriteLog(Priority priority, int isLogEnabled, const std::string message = "", bool calculatedTime = false, long long prevTime = 0);
	static void WriteFileLog(Priority priority, int isLogEnabled, const std::string message);
	static void WriteFileLogNew(int isLogEnabled, const std::string message);

	static long long GetTimeDifference(long long prevTime);

private:

	ofstream    fileStream;
	Priority    maxPriority;
	std::string		logFile;

	static const std::string PRIORITY_NAMES[];
	static CLogPrinter instance;
    static bool isLogEnable;

	//static SmartPointer<CLockHandler> m_pLogPrinterMutex;

};



#ifdef LOG_ENABLED
#define CLogPrinter_WriteLog(...) CLogPrinter::WriteLog(__VA_ARGS__)
#else
#define CLogPrinter_WriteLog(...) 0
#endif

#ifdef LOG_ENABLED
#define CLogPrinter_LOG(...) CLogPrinter::LogWithCheck(__VA_ARGS__)
#else
#define CLogPrinter_LOG(...) 0
#endif

#ifdef LOG_ENABLED
#define CLogPrinter_WriteFileLog(...) CLogPrinter::WriteFileLog(__VA_ARGS__)
#else
#define CLogPrinter_WriteFileLog(...) 
#endif

#ifdef __PRINT_LOG__
#define CLogPrinter_Write(...) CLogPrinter::Write(__VA_ARGS__)
#else
#define CLogPrinter_Write(...)
#endif

#ifdef __SPECIFIC_LOG__
#define CLogPrinter_WriteSpecific(...) CLogPrinter::WriteSpecific(__VA_ARGS__)
#else
#define CLogPrinter_WriteSpecific(...)
#endif

#ifdef __SPECIFIC_LOG2__
#define CLogPrinter_WriteSpecific2(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteSpecific2(...)
#endif

#ifdef __SPECIFIC_LOG3__
#define CLogPrinter_WriteSpecific3(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteSpecific3(...)
#endif

#ifdef __SPECIFIC_LOG6__
#define CLogPrinter_WriteSpecific6(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteSpecific6(...)
#endif

#ifdef __INSTENT_TEST_LOG__
#define CLogPrinter_WriteInstentTestLog(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteInstentTestLog(...)
#endif

#ifdef __SPECIFIC_LOG4__
#define CLogPrinter_WriteSpecific4(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteSpecific4(...)
#endif

#ifdef __SPECIFIC_LOG5__
#define CLogPrinter_WriteSpecific5(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteSpecific5(...)
#endif

#ifdef __THREAD_LOG__
#define CLogPrinter_WriteThreadLog(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteThreadLog(...)
#endif

#ifdef __OPERATION_TIME_LOG__
#define CLogPrinter_WriteForOperationTime(...) CLogPrinter::WriteForOperationTime(__VA_ARGS__)
#else
#define CLogPrinter_WriteForOperationTime(...) 0
#endif

#ifdef __QUEUE_TIME_LOG__
#define CLogPrinter_WriteForQueueTime(...) CLogPrinter::WriteForQueueTime(__VA_ARGS__)
#else
#define CLogPrinter_WriteForQueueTime(...)
#endif

#ifdef __PACKET_LOSS_INFO_LOG__
#define CLogPrinter_WriteForPacketLossInfo(...) CLogPrinter::WriteForPacketLossInfo(__VA_ARGS__)
#else
#define CLogPrinter_WriteForPacketLossInfo(...)
#endif



#ifdef __BITRATE_CHNANGE_LOG__
#define CLogPrinter_WriteBitrateChangeInfo(...) CLogPrinter::WriteSpecific2(__VA_ARGS__)
#else
#define CLogPrinter_WriteBitrateChangeInfo(...)
#endif

} //namespace MediaSDK


#endif
