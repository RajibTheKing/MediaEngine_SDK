#include "LogPrinter.h"
#include "ThreadTools.h"

namespace MediaSDK
{

	const std::string CLogPrinter::PRIORITY_NAMES[] =
	{
		"NONE",
		"DEBUGS",
		"CONFIG",
		"INFO",
		"WARNING",
		"ERRORS"
	};

	CLogPrinter CLogPrinter::instance;

	bool CLogPrinter::isLogEnable;

	CLogPrinter::CLogPrinter()
	{
		Priority maxPriority = CLogPrinter::NONE;
		std::string logFile = "/sdcard/DefaultMediaEnfineLog.txt";

		isLogEnable = false;


#ifdef PRIORITY
		maxPriority = PRIORITY;
#endif 
#ifdef FILE_NAME
		logFile = FILE_NAME;
#endif 

		//m_pLogPrinterMutex.reset(new CLockHandler);

		/*if (logFile != "")
		{
		instance.maxPriority = maxPriority;
		instance.logFile = logFile;
		instance.fileStream.open(logFile.c_str());
		}
		else
		instance.maxPriority = CLogPrinter::NONE;
		*/
	}

	void CLogPrinter::Start(Priority maxPriority, const char* logFile)
	{
		const char* tempLogFile = logFile;
		if ("" == logFile)
		{
			tempLogFile = FILE_NAME;
		}
		else
		{
			tempLogFile = logFile;
		}

		if (tempLogFile != "")
		{
			instance.maxPriority = maxPriority;
			instance.logFile = tempLogFile;

#ifdef	LOG_ENABLED

			instance.fileStream.open(tempLogFile, ofstream::out);

#endif

		}
		else
			instance.maxPriority = CLogPrinter::NONE;
	}

	CLogPrinter::~CLogPrinter()
	{
		//SHARED_PTR_DELETE(m_pLogPrinterMutex);
	}

	void CLogPrinter::SetLoggerPath(std::string loc)
	{
		instance.logFile = loc;

		if (instance.fileStream.is_open())
			instance.fileStream.close();

#ifdef	LOG_ENABLED

		instance.fileStream.open(loc.c_str(), ofstream::out);

#endif

	}

	bool CLogPrinter::SetLoggingState(bool loggingState, int logLevel)
	{
		isLogEnable = loggingState;

		return true;
	}

	void CLogPrinter::Stop()
	{
		if (instance.fileStream.is_open())
		{
			instance.fileStream.close();
		}
	}

	void CLogPrinter::Argument_to_String(string &dst, const char *format, va_list ap)
	{
		int length;
		va_list apDuplicate;
		va_copy(apDuplicate, ap);
		length = vsnprintf(NULL, 0, format, apDuplicate);
		va_end(apDuplicate);

		if (length > 0)
		{
			dst.resize(length);
			vsnprintf((char *)dst.data(), dst.size() + 1, format, ap);
		}
		else
		{
			dst = "Format error! format: ";
			dst.append(format);
		}
	}

	void CLogPrinter::Log(const char *format, ...)
	{
		std::string str;
		va_list vargs;
		va_start(vargs, format);
		Argument_to_String(str, format, vargs);
		va_end(vargs);


#if defined(__ANDROID__)
		LOGE("%s", str.c_str());
#elif defined(TARGET_OS_WINDOWS_PHONE) || defined(DESKTOP_C_SHARP) || defined(TARGET_OS_IPHONE)
		printf("%s\n", str.c_str());
#endif

	}

	void CLogPrinter::SetPriority(Priority maxPriority)
	{
		instance.maxPriority = maxPriority;
	}

	void CLogPrinter::Write(Priority priority, const std::string message)
	{

		if (isLogEnable)
		{

#ifdef __PRINT_LOG__

#ifdef __EXACT_LOG__

			if (priority == instance.minPriority)
			{
				ostream& stream = instance.fileStream.is_open() ? instance.fileStream : std::cout;

				stream << getDateTime() << PRIORITY_NAMES[priority] << ": " << message << endl;
			}
#else
			//if (priority <= instance.maxPriority)
		{
			//ostream& stream = instance.fileStream.is_open() ? instance.fileStream : std::cout;

			//stream << GetDateTime() << PRIORITY_NAMES[priority] << ": " << message << endl;

#if defined(TARGET_OS_IPHONE)

			cout<<PRIORITY_NAMES[priority] << "--> "<<message<<endl;
#elif defined(TARGET_OS_WINDOWS_PHONE) || defined(DESKTOP_C_SHARP)
			printf("%s ---> %s\n",PRIORITY_NAMES[priority].c_str(), message.c_str());
#elif defined(__ANDROID__)

			LOGE("%s ---> %s ",PRIORITY_NAMES[priority].c_str(), message.c_str());

#endif

		}

#endif

#endif 

		}
	}

	void CLogPrinter::WriteSpecific(Priority priority, const std::string message)
	{

		//if(isLogEnable)
		{

#ifdef __SPECIFIC_LOG__


#if defined(TARGET_OS_IPHONE)
			cout<<PRIORITY_NAMES[priority] << "--> "<<message<<endl;
#elif defined(TARGET_OS_WINDOWS_PHONE) || defined(DESKTOP_C_SHARP)
			printf("%s ---> %s\n",PRIORITY_NAMES[priority].c_str(), message.c_str());
#elif defined(__ANDROID__)

			LOGE("%s ---> %s ",PRIORITY_NAMES[priority].c_str(), message.c_str());

#endif

#endif

		}
	}

	void CLogPrinter::WriteSpecific2(Priority priority, const std::string message)
	{

		//if(isLogEnable)
		{



#if defined(TARGET_OS_IPHONE)
			cout<<PRIORITY_NAMES[priority] << "--> "<<message<<endl;
#elif defined(TARGET_OS_WINDOWS_PHONE) || defined(DESKTOP_C_SHARP)
			printf("%s ---> %s\n", PRIORITY_NAMES[priority].c_str(), message.c_str());
#elif defined(__ANDROID__)

			LOGE("%s ---> %s ",PRIORITY_NAMES[priority].c_str(), message.c_str());

#endif


		}
	}

	std::string CLogPrinter::GetDateTime()
	{

#ifdef _WIN32

		SYSTEMTIME st;

		GetSystemTime(&st);

		char currentTime[103] = "";

		sprintf(currentTime, "[%02d-%02d-%04d %02d:%02d:%02d:%03d] ", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		return std::string(currentTime);

#else

		timeval curTime;
		gettimeofday(&curTime, NULL);
		int milli = curTime.tv_usec / 1000;

		char buffer[103];
		strftime(buffer, 80, "[%d-%m-%Y %H:%M:%S", localtime(&curTime.tv_sec));

		char currentTime[103] = "";
		sprintf(currentTime, "%s:%d] ", buffer, milli);

		return std::string(currentTime);

#endif 

	}

	long long CLogPrinter::WriteForOperationTime(Priority priority, const std::string message, long long prevTime)
	{
#ifdef __OPERATION_TIME_LOG__
		long long timeDiff = 0;
		if(isLogEnable)
		{
			timeDiff = GetTimeDifference(prevTime);
			if(prevTime==0) return timeDiff;
#if defined(TARGET_OS_IPHONE)

			cout<< "OperatTime: " << timeDiff <<" :" << "--> "<<message<<endl;

#elif defined(__ANDROID__)

			LOGE("OperatTime: %lld ---> %s ", timeDiff, message.c_str());
#elif defined(DESKTOP_C_SHARP)
			printf("OperatTimeLog: %lld %s\n", timeDiff, message.c_str());
#endif

		}

		return timeDiff;
#endif
		return -1;
	}

	long long CLogPrinter::WriteLog(Priority priority, int isLogEnabled, const std::string message, bool calculatedTime, long long prevTime)
	{
		if (isLogEnabled)
		{
			if (calculatedTime)
			{
				long long timeDiff = 0;

				timeDiff = GetTimeDifference(prevTime);

				if (prevTime == 0)
					return timeDiff;

#if defined(__ANDROID__)

				LOGE("%s :: %s --> %lld", PRIORITY_NAMES[priority].c_str(), message.c_str(), timeDiff);

#elif defined(TARGET_OS_WINDOWS_PHONE) || defined(DESKTOP_C_SHARP) || defined(TARGET_OS_IPHONE)

				printf("%s :: %s --> %lld\n", PRIORITY_NAMES[priority].c_str(), message.c_str(), timeDiff);

#endif

				return timeDiff;
			}
			else
			{

#if defined(__ANDROID__)

				LOGE("%s :: %s", PRIORITY_NAMES[priority].c_str(), message.c_str());

#elif defined(TARGET_OS_WINDOWS_PHONE) || defined(DESKTOP_C_SHARP) || defined(TARGET_OS_IPHONE)

				printf("%s :: %s\n", PRIORITY_NAMES[priority].c_str(), message.c_str());

#endif
			}

		}

		return -1;
	}

	void CLogPrinter::WriteFileLog(Priority priority, int isLogEnabled, const std::string message)
	{
		//Locker lock(*m_pLogPrinterMutex);

#ifdef __ANDROID__

		if(isLogEnabled)
		{

			ostream& stream = instance.fileStream.is_open() ? instance.fileStream : std::cout;
			stream << GetDateTime() << PRIORITY_NAMES[priority] << ": " << message << endl;

		}

#endif

	}

	void CLogPrinter::WriteForQueueTime(Priority priority, const std::string message)
	{
		if (isLogEnable)
		{

#ifdef __QUEUE_TIME_LOG__


#if defined(TARGET_OS_IPHONE)

			cout<< "QueueTime: " << PRIORITY_NAMES[priority] << "--> "<<message<<endl;

#elif defined(__ANDROID__)

			LOGE("QueueTime: %s ---> %s ",PRIORITY_NAMES[priority].c_str(), message.c_str());

#elif defined(DESKTOP_C_SHARP)
			printf("QueueTime:  %s\n", message.c_str());
#endif

#endif

		}
	}
	void CLogPrinter::WriteForPacketLossInfo(Priority priority, const std::string message)
	{
		if (isLogEnable)
		{

#ifdef __PACKET_LOSS_INFO_LOG__


#if defined(TARGET_OS_IPHONE)

			cout<<"PacketLoss: " << PRIORITY_NAMES[priority] << "--> "<<message<<endl;

#elif defined(__ANDROID__)

			LOGE("PacketLoss: %s ---> %s ",PRIORITY_NAMES[priority].c_str(), message.c_str());
#elif defined(DESKTOP_C_SHARP)
			printf("PacketLoss: %s\n", message.c_str());

#endif

#endif

		}
	}

	long long  CLogPrinter::GetTimeDifference(long long prevTime)
	{

#ifndef USE_CPP_11_TIME

#if defined(TARGET_OS_WINDOWS_PHONE) || defined(DESKTOP_C_SHARP)
		return GetTickCount64();
#else
		struct timeval te;
		gettimeofday(&te, NULL); // get current time
		long long milliseconds =  te.tv_sec* + te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
		// printf("milliseconds: %lld\n", milliseconds);
		return milliseconds - prevTime;
#endif

#else


		namespace sc = std::chrono;

		auto time = sc::system_clock::now(); // get the current time

		auto since_epoch = time.time_since_epoch(); // get the duration since epoch

		// I don't know what system_clock returns
		// I think it's uint64_t nanoseconds since epoch
		// Either way this duration_cast will do the right thing
		auto millis = sc::duration_cast<sc::milliseconds>(since_epoch);

		long long now = millis.count(); // just like java (new Date()).getTime();

		return now - prevTime;
#endif
	}





} //namespace MediaSDK



