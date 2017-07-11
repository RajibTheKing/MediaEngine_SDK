#include "MediaLogger.h"

namespace MediaSDK
{
	MediaLogger::MediaLogger(LogLevel logLevel) :
		m_elogLevel(logLevel)
	{		
		m_sFilePath = MEDIA_FULL_LOGGING_PATH;
	}

	MediaLogger::~MediaLogger()
	{
		Release();
	}

	void MediaLogger::Init()
	{
		m_pMediaLoggerMutex.reset(new CLockHandler());

		if (!m_pLoggerFileStream.is_open())
		{
			m_pLoggerFileStream.open(m_sFilePath.c_str(), ofstream::out | ofstream::app);
		}
		
		StartMediaLoggingThread();
	}
	
	void MediaLogger::Release()
	{
		StopMediaLoggingThread();

		if (m_pLoggerFileStream.is_open())
		{
			m_pLoggerFileStream.close();
		}
	}

	std::string MediaLogger::GetFilePath()
	{
		return m_sFilePath;
	}

	LogLevel MediaLogger::GetLogLevel()
	{
		return m_elogLevel;
	}

	void MediaLogger::Log(LogLevel logLevel, const char *format, ...)
	{
		MediaLocker lock(m_pMediaLoggerMutex.get());

		if (logLevel > m_elogLevel) return;

		va_list vargs;
		//argument to string start
		va_start(vargs, format);
		vsnprintf(m_sMessage, MEDIA_LOG_MAX_SIZE, format, vargs);
		va_end(vargs);
		//argument to string end
		
		m_vLogVector.push_back(GetDateTime() + GetThreadID() + " " + m_sMessage);
		CLogPrinter::Log("MANSUR----------pushing >> %s\n", m_sMessage);
	}
	void MediaLogger::WriteLogToFile()
	{
		MediaLocker lock(m_pMediaLoggerMutex.get());

		size_t vSize = min(m_vLogVector.size(), (size_t)MAX_BUFFERED_LOG);

		for(int  i=0; i < vSize; i++)
		{
			CLogPrinter::Log("MANSUR----------writing to file stream >> %s\n", m_vLogVector[i].c_str());
			m_pLoggerFileStream << m_vLogVector[i] << std::endl;
		}

		m_vLogVector.erase(m_vLogVector.begin(), m_vLogVector.begin() + vSize);
	}

	std::string MediaLogger::GetThreadID()
	{
		//For All Platforms
		stringstream ss;
		ss << std::this_thread::get_id();
		return ss.str();
	}

	std::string MediaLogger::GetDateTime()
	{
		stringstream ss;
		ss<<std::time(nullptr);

#if defined(DESKTOP_C_SHARP) || defined(TARGET_OS_WINDOWS_PHONE)

		SYSTEMTIME st;

		GetSystemTime(&st);

		char currentTime[40] = "";

		_snprintf_s(currentTime, 40, "[%02d-%02d-%04d %02d:%02d:%02d: %03s] ", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, ss.str().c_str());

		return std::string(currentTime);

#else

		timeval curTime;
		gettimeofday(&curTime, NULL);
		int milli = curTime.tv_usec / 1000, pos;

		char buffer[42];
		pos = strftime(buffer, 22, "[%d-%m-%Y %H:%M:%S", localtime(&curTime.tv_sec));
		CLogPrinter::Log("MANSUR---------position value = %d\n", pos);
		snprintf(buffer+pos, 20, " %s] ", ss.str().c_str());

		return std::string(buffer);

#endif 

	}

	void MediaLogger::StartMediaLoggingThread()
	{
		m_threadInstance = std::thread(CreateLoggingThread, this);
	}
	
	void MediaLogger::StopMediaLoggingThread()
	{
		CLogPrinter::Log("MANSUR----------stopping logger thread\n");
		m_bMediaLoggingThreadRunning = false;
		
		m_threadInstance.join();
	}

	void *MediaLogger::CreateLoggingThread(void* param)
	{
		MediaLogger *pThis = (MediaLogger*)param;

		pThis->m_bMediaLoggingThreadRunning = true;

		CLogPrinter::Log("MANSUR----------starting logger thread\n");

		while (pThis->m_bMediaLoggingThreadRunning)
		{
			if (pThis->m_vLogVector.size() >= MIN_BUFFERED_LOG)
			{
				pThis->WriteLogToFile();
			}
			else
			{ 
				CLogPrinter::Log("MANSUR----------sleeping logger thread , vector size = %d\n", pThis->m_vLogVector.size());
				Tools::SOSleep(THREAD_SLEEP_TIME);
			}
		}

		return NULL;
	}

}
