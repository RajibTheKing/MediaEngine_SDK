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
		/*if (m_pLoggerFileStream)
		{
			fclose(m_pLoggerFileStream);
		}*/

		m_vLogVector.clear();
		if (m_pLoggerFileStream.is_open()) m_pLoggerFileStream.close();
	}

	void MediaLogger::Init()
	{
		/*if (m_pLoggerFileStream)
		{
			fclose(m_pLoggerFileStream);
			//m_pLoggerFileStream = nullptr;
		}*/
		//m_pLoggerFileStream = fopen(m_sFilePath.c_str(), "w");
		if (!m_pLoggerFileStream.is_open())
		{
			m_pLoggerFileStream.open(m_sFilePath.c_str(), ofstream::out | ofstream::app);
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
		//MediaLocker lock(*m_pMediaLoggerMutex);

		if (logLevel > m_elogLevel) return;

		va_list vargs;
		//argument to string start
		va_start(vargs, format);
		vsnprintf(m_sMessage, MEDIA_LOG_MAX_SIZE, format, vargs);
		va_end(vargs);
		//argument to string end
		
		m_vLogVector.push_back(GetDateTime() + GetThreadID() + m_sMessage);
		for (int i = 0; i < m_vLogVector.size(); i++)
        {
            CLogPrinter::Log("MANSUR----------log>> %s\n",m_vLogVector[i].c_str());
        }
		
	}
	void MediaLogger::WriteLogToFile()
	{
		//MediaLocker lock(*m_pMediaLoggerMutex);

		if (!m_pLoggerFileStream.is_open())
		{
			m_pLoggerFileStream.open(m_sFilePath.c_str(), ofstream::out | ofstream::app);
		}
		for(int  i=0; i<m_vLogVector.size(); i++)
		{
			CLogPrinter::Log("MANSUR----------writing to file stream >> %s\n", m_vLogVector[i].c_str());
			m_pLoggerFileStream << m_vLogVector[i] << std::endl;
		}

	}
	std::string MediaLogger::GetThreadID()
	{
		//For All Platforms
		std::string str;
		uint64_t iThreadID;
		auto id = std::this_thread::get_id();
		uint64_t* ptr = (uint64_t*)&id;
		iThreadID = *ptr;

		stringstream ss;
		ss << iThreadID;
		ss >> str;
		return str;
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


}
