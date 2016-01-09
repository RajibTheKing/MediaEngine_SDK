#include "LogPrinter.h"

#ifdef __ANDROID__

#include <android/log.h>

#define LOG_TAG "LibraryLog"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#endif

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
	std::string logFile = "";
    
    isLogEnable = false;
    

#ifdef PRIORITY
	maxPriority = PRIORITY;
#endif 
#ifdef FILE_NAME
	logFile = FILE_NAME;
#endif 

	//if (logFile != "")
	//{
	//	instance.maxPriority = maxPriority;
	//	instance.logFile = logFile;
	//	instance.fileStream.open(logFile.c_str());
	//}
	//else
	//	instance.maxPriority = CLogPrinter::NONE;
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
		instance.fileStream.open(tempLogFile);
	}
	else
		instance.maxPriority = CLogPrinter::NONE;
}

void CLogPrinter::SetLoggerPath(std::string loc)
{
	instance.logFile = loc;

	if (instance.fileStream.is_open())
		instance.fileStream.close();

	instance.fileStream.open(loc.c_str());
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

void CLogPrinter::SetPriority(Priority maxPriority)
{
	instance.maxPriority = maxPriority;
}

void CLogPrinter::Write(Priority priority, const std::string message)
{
//	printf("%s\n", message.c_str());
    
    if(isLogEnable)
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

#ifdef TARGET_OS_IPHONE
            
            cout<<PRIORITY_NAMES[priority] << "--> "<<message<<endl;

#elif __ANDROID__

            LOGE("%s ---> %s ",PRIORITY_NAMES[priority].c_str(), message.c_str());

#endif

        }

#endif

#endif 
        
    }
}

void CLogPrinter::WriteSpecific(Priority priority, const std::string message)
{
    if(isLogEnable)
    {
        
#ifdef __SPECIFIC_LOG__
    

#ifdef TARGET_OS_IPHONE

        cout<<PRIORITY_NAMES[priority] << "--> "<<message<<endl;

#elif __ANDROID__

        LOGE("%s ---> %s ",PRIORITY_NAMES[priority].c_str(), message.c_str());

#endif
    
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







