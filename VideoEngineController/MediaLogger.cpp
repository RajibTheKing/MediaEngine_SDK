#include "MediaLogger.h"
#include "Tools.h"
#include "LogPrinter.h"

#if defined(TARGET_OS_IPHONE) || defined(__ANDROID__) || defined(TARGET_IPHONE_SIMULATOR)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace MediaSDK
{
	MediaLogger::MediaLogger():
		m_elogLevel(LogLevel::INFO), //by default
		m_bFSError(false)
	{
		m_pMediaLoggerMutex.reset(new CLockHandler());

		m_sFilePath = MEDIA_FULL_LOGGING_PATH;

		InternalLog(">>>>> Media SDK Logging Started <<<<<");
		InternalLog("Media SDK version %s [%lu]", MEDIA_ENGINE_VERSION, MEDIA_ENGINE_BUILD_NUMBER);
	}

	MediaLogger::~MediaLogger()
	{
		Release();
	}

	void MediaLogger::Init(LogLevel logLevel)
	{
		m_elogLevel = logLevel;
		
		InternalLog("Media SDK Logging Level %d", m_elogLevel);

		m_bFSError = !CreateLogDirectory(); //if TRUE then NO ERROR, else we have FS error

		if (!m_bFSError) //no error then
		{
			if (!m_pLoggerFileStream.is_open())
			{
				m_pLoggerFileStream.open(m_sFilePath.c_str(), ofstream::out | ofstream::app);
			}
		}
		
		StartMediaLoggingThread();
	}
	
	void MediaLogger::Release()
	{
		InternalLog(">>>>> Media SDK Logging Finished <<<<<");

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
		if (logLevel > m_elogLevel) return;

		int len = GetDateTime(m_sMessage);
		len += GetThreadID(m_sMessage + len);
		m_sMessage[len++] = ' ';

		va_list vargs;
		//argument to string start
		va_start(vargs, format);
		vsnprintf(m_sMessage + len, MEDIA_LOG_MAX_SIZE - len, format, vargs);
		va_end(vargs);
		//argument to string end

		MediaLocker lock(m_pMediaLoggerMutex.get());
		
		m_vLogVector.push_back(m_sMessage);
	}

	void MediaLogger::InternalLog(const char *format, ...)
	{
		int len = GetDateTime(m_sMessage);
		len += GetThreadID(m_sMessage + len);
		m_sMessage[len++] = ' ';

		va_list vargs;
		//argument to string start
		va_start(vargs, format);
		vsnprintf(m_sMessage + len, MEDIA_LOG_MAX_SIZE - len, format, vargs);
		va_end(vargs);
		//argument to string end

		MediaLocker lock(m_pMediaLoggerMutex.get());

		m_vLogVector.push_back(m_sMessage);
	}

	bool MediaLogger::CreateLogDirectory()
	{
#if defined(DESKTOP_C_SHARP) || defined(TARGET_OS_WINDOWS_PHONE)

		if (0 == CreateDirectory(MEDIA_LOGGING_PATH, NULL) )
		{
			int retCode = GetLastError();
			if (ERROR_ALREADY_EXISTS != retCode)
			{
				printf("[%s] Log folder creation FAILED, code %d\n", MEDIA_LOGGER_TAG, retCode);

				return false;
			}
		}

#else
		struct stat st = { 0 };
        
        const char* pathArray;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
        pathArray = (MEDIA_LOGGING_PATH).c_str();
#else
        pathArray = MEDIA_LOGGING_PATH;
#endif
        
		if (stat(pathArray, &st) == -1)
		{
			if(0 != mkdir(pathArray, 0700))
			{

#if __ANDROID__

				__android_log_print(ANDROID_LOG_ERROR, MEDIA_LOGGER_TAG, "Log folder creation FAILED, code %d\n", errno);

#elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

				printf("[%s] Log folder creation FAILED, code %d\n", MEDIA_LOGGER_TAG, errno);

#endif

				return false;
			}
		}
#endif

		return true;
	}

	void MediaLogger::WriteLogToFile()
	{
		MediaLocker lock(m_pMediaLoggerMutex.get());

		size_t vSize = min(m_vLogVector.size(), (size_t)MAX_LOG_WRITE);

		auto vPos = m_vLogVector.begin();

		for(; vSize>0; vSize--, vPos++)
		{
			if (!m_bFSError)
			{
				m_pLoggerFileStream << *vPos << std::endl;
			}
			else
			{

#if defined(DESKTOP_C_SHARP) || defined(TARGET_OS_WINDOWS_PHONE) || defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

				std::cout << *vPos << std::endl;

#elif __ANDROID__

				__android_log_write(ANDROID_LOG_ERROR, MEDIA_LOGGER_TAG, vPos->c_str());

#endif
			}
		}

		m_pLoggerFileStream.flush();

		m_vLogVector.erase(m_vLogVector.begin(), vPos);

		//Check that the file is not getting too big
		if (m_pLoggerFileStream.tellp() >= MAX_LOG_FILE_SIZE_BYTES)
		{
			RenameFile();
		}
	}

	void MediaLogger::RenameFile()
	{
		//Close the file stream and rename
		m_pLoggerFileStream.close();

		char currentDateTime[45];
		GetDateTime(currentDateTime);

		string newFile(MEDIA_LOGGING_FILE_NAME);

		//Date
		newFile += strtok(currentDateTime, " []");

		newFile += "_";

		//Time
		newFile += strtok(NULL, " []") + string(MEDIA_LOGGING_FILE_EXT);

		//Now replace all the : on the time portion with "_"
		auto colonPos = newFile.find(":");

		while (colonPos != string::npos)
		{
			newFile.replace(colonPos, 1, "_");
			colonPos = newFile.find(":");
		}
		
		/// After replacing all ":", now add the path
		/// Adding path before will replace ":" after drive letter in Windows
		newFile.insert(0, MEDIA_LOGGING_PATH);

		//delete the old one, if any
		//std::remove(newFile.c_str());

		//Rename
		if (0 != std::rename(m_sFilePath.c_str(), newFile.c_str()))
		{
			//Rename fails, so wait a sec to get a unique name
			Log(LOG_ERROR, "[%s] Rename to file %s\n FAILED", MEDIA_LOGGER_TAG, newFile.c_str());
		}

		//Reopen
		m_pLoggerFileStream.open(m_sFilePath.c_str(), ofstream::out | ofstream::app);
	}

	size_t MediaLogger::GetThreadID(char* buffer)
	{
#if defined(__ANDROID__)
        return snprintf(buffer, 15, "%d", gettid());
#else
		
#endif
	//	//For All Platforms
		stringstream ss;
		ss << std::this_thread::get_id();
		ss >> buffer;
        return strlen(buffer);
	}

	size_t MediaLogger::GetDateTime(char* buffer)
	{
		unsigned long long epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

#if defined(DESKTOP_C_SHARP) || defined(TARGET_OS_WINDOWS_PHONE)

		SYSTEMTIME st;

		GetLocalTime(&st);

		return _snprintf(buffer, 40, "[%02d-%02d-%04d %02d:%02d:%02d %llu] ", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, epoch);
#else

		timeval curTime;
		gettimeofday(&curTime, NULL);
		int milli = curTime.tv_usec / 1000, pos;

		pos = strftime(buffer, 22, "[%d-%m-%Y %H:%M:%S", localtime(&curTime.tv_sec));
		pos += snprintf(buffer + pos, 20, " %llu] ", epoch);

		return pos;
#endif 
	}

	void MediaLogger::StartMediaLoggingThread()
	{
		if (!m_pThreadInstance.get())
		{
			m_pThreadInstance.reset(new std::thread(CreateLoggingThread, this));
		}
	}
	
	void MediaLogger::StopMediaLoggingThread()
	{
		InternalLog("Stopping logger thread...");
		m_bMediaLoggingThreadRunning = false;
		
		m_pThreadInstance->join();
	}

	void *MediaLogger::CreateLoggingThread(void* param)
	{
		MediaLogger *pThis = (MediaLogger*)param;

		pThis->m_bMediaLoggingThreadRunning = true;

		pThis->InternalLog("Starting logger thread");

		while (pThis->m_bMediaLoggingThreadRunning)
		{
			if (pThis->m_vLogVector.size() >= MIN_BUFFERED_LOG)
			{
				pThis->WriteLogToFile();
			}
			else
			{ 
				//InternalLog("Sleeping logger thread , vector size = %d\n", pThis->m_vLogVector.size());
				Tools::SOSleep(THREAD_SLEEP_TIME);
			}
		}

		//When we will exit, we want to write everything pending
		pThis->WriteLogToFile();

		return NULL;
	}

}
