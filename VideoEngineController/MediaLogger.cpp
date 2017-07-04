#include "MediaLogger.h"

namespace MediaSDK
{
	MediaLogger::MediaLogger(std::string filePath, LogLevel logLevel) :
		m_pLoggerFile(nullptr),
		m_elogLevel(logLevel)
	{		
#if defined(__ANDROID__)
		m_sFilePath = "/sdcard/"+filePath;
#elif defined(DESKTOP_C_SHARP)
		m_sFilePath = filePath;
#endif
	}

	MediaLogger::~MediaLogger()
	{
		if (m_pLoggerFile)
		{
			fclose(m_pLoggerFile);
		}
		m_vLogVector.clear();
	}

	void MediaLogger::Init()
	{
		/*if (m_pLoggerFile)
		{
			fclose(m_pLoggerFile);
			//m_pLoggerFile = nullptr;
		}*/
		m_pLoggerFile = fopen(m_sFilePath.c_str(), "w");
	}

	inline std::string MediaLogger::GetFilePath()
	{
		return m_sFilePath;
	}

	inline MediaLogger::LogLevel MediaLogger::GetLogLevel()
	{
		return m_elogLevel;
	}
	void MediaLogger::Log(LogLevel logLevel, const char *format, ...)
	{
		MediaLocker lock(*m_pMediaLoggerMutex);

		if (logLevel > m_elogLevel) return;

		std::string str;
		va_list vargs;
		va_start(vargs, format);

		//argument to string start
		int length;
		va_list apDuplicate;
		va_copy(apDuplicate, vargs);
		length = vsnprintf(NULL, 0, format, apDuplicate);
		va_end(apDuplicate);

		if (length > 0)
		{
			str.resize(length);
			vsnprintf((char *)str.data(), str.size() + 1, format, vargs);
		}
		else
		{
			str = "Format error! format: ";
			str.append(format);
		}
		//argument to string end

		va_end(vargs);
		
		m_vLogVector.push_back(GetDateTime() + " " + GetThreadId() + " " + " " + str);
	}
	void MediaLogger::WriteLogToFile()
	{
		//Locker lock(*m_pLogPrinterMutex);

#if defined(__ANDROID__)

			ostream& stream = instance.fileStream.is_open() ? instance.fileStream : std::cout;
			stream << GetDateTime() << PRIORITY_NAMES[priority] << ": " << message << endl;

#endif

	}
	std::string MediaLogger::GetDateTime()
	{

#if defined(DESKTOP_C_SHARP) || defined(TARGET_OS_WINDOWS_PHONE)

		SYSTEMTIME st;

		GetSystemTime(&st);

		char currentTime[103] = "";

		sprintf(currentTime, "[%02d-%02d-%04d %02d:%02d:%02d: %03d] ", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		return std::string(currentTime);

#else

		timeval curTime;
		gettimeofday(&curTime, NULL);
		int milli = curTime.tv_usec / 1000;

		char buffer[103];
		strftime(buffer, 80, "[%d-%m-%Y %H:%M:%S", localtime(&curTime.tv_sec));

		char currentTime[103] = "";
		sprintf(currentTime, "%s: %d] ", buffer, milli);

		return std::string(currentTime);

#endif 

	}
	std::string GetThreadId()
	{
		int iThreadID;
#if defined(DESKTOP_C_SHARP) || defined(TARGET_OS_WINDOWS_PHONE)
		iThreadID = GetCurrentThreadId();
#else
		iThreadID = pthread_self();
#endif
		return std::to_string(iThreadID);
	}
}