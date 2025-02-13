
#include "Tools.h"

#include <sstream>
#include <cstdlib>
#include "LogPrinter.h"

namespace MediaSDK
{

	Tools::Tools()
	{
		m_filePointerToWriteByteData = NULL;
		m_filePointerToWriteShortData = NULL;
#ifdef DESKTOP_C_SHARP
		timeBeginPeriod(1);
#endif
	}

	Tools::~Tools()
	{
#ifdef DESKTOP_C_SHARP
		timeEndPeriod(16);
#endif
		if (NULL != m_filePointerToWriteByteData)
			fclose(m_filePointerToWriteByteData);

		if (NULL != m_filePointerToWriteShortData)
			fclose(m_filePointerToWriteShortData);
	}

	std::string Tools::DoubleToString(double dConvertingValue)
	{
		stringstream convertedStringStream;

		convertedStringStream << dConvertingValue;

		return convertedStringStream.str();
	}

	std::string Tools::IntegertoStringConvert(int nConvertingNumber)
	{
		char cConvertedCharArray[12];

#ifdef _WIN32

		_itoa_s(nConvertingNumber, cConvertedCharArray, 10);

#else

		sprintf(cConvertedCharArray, "%d", nConvertingNumber);

#endif

		return (std::string)cConvertedCharArray;
	}

	std::string Tools::LongLongtoStringConvert(long long number)
	{
		char buf[22];

		int i, j, k;

		bool negative = false;

		if (number < (long long)0)
		{
			number *= (long long)-1;
			negative = true;
		}

		for (i = 0; number; i++)
		{
			buf[i] = number % 10 + '0';
			number /= 10;
		}

		if (negative)
		{
			buf[i++] = '-';
		}

		for (j = i - 1, k = 0; k < j; k++, j--)
		{
			buf[j] ^= buf[k];
			buf[k] ^= buf[j];
			buf[j] ^= buf[k];
		}

		buf[i] = '\0';

		return (std::string)buf;
	}


	std::string Tools::getText(int nConvertingNumber)
	{
		char cConvertedCharArray[12];

#ifdef _WIN32

		_itoa_s(nConvertingNumber, cConvertedCharArray, 10);

#else

		sprintf(cConvertedCharArray, "%d", nConvertingNumber);

#endif

		return (std::string)cConvertedCharArray;
	}

	std::string Tools::getText(long long number)
	{
		char buf[22];

		if (number == 0) {
			return "0";
		}

		int i, j, k;

		bool negative = false;

		if (number < (long long)0)
		{
			number *= (long long)-1;
			negative = true;
		}

		for (i = 0; number; i++)
		{
			buf[i] = number % 10 + '0';
			number /= 10;
		}

		if (negative)
		{
			buf[i++] = '-';
		}

		for (j = i - 1, k = 0; k < j; k++, j--)
		{
			buf[j] ^= buf[k];
			buf[k] ^= buf[j];
			buf[j] ^= buf[k];
		}

		buf[i] = '\0';

		return (std::string)buf;
	}

	std::string Tools::getText(unsigned long long number)
	{
		char buf[22];
		sprintf(buf, "%llu", number);
		return (std::string)buf;
	}

	std::string Tools::getText(double dConvertingValue)
	{
		stringstream convertedStringStream;

		convertedStringStream << dConvertingValue;

		return convertedStringStream.str();
	}

	int Tools::StringToIntegerConvert(std::string strConvertingString)
	{
		int nConvertedNumber;

		nConvertedNumber = atoi(strConvertingString.c_str());

		return nConvertedNumber;
	}

	std::string Tools::LongLongToString(long long llConvertingValue)
	{
		stringstream convertedStringStream;

		convertedStringStream << llConvertingValue;

		return convertedStringStream.str();
	}

	void Tools::ConvertToCharArray(unsigned char *arr, unsigned long long a, int len)
	{
		int i = 0;  
		for (i = 0; i < len; ++i) 
		{
			arr[i] = (unsigned char)((a >> (len - 1 - i) * 8) & 0xFFu);
		}
	}

	void Tools::SOSleep(int nSleepTimeout)
	{

#ifdef _WIN32
		Sleep(nSleepTimeout);
#else

		timespec t;

		u_int32_t seconds = nSleepTimeout / 1000;
		t.tv_sec = seconds;
		t.tv_nsec = (nSleepTimeout - (seconds * 1000)) * (1000 * 1000);

		nanosleep(&t, NULL);

#endif

	}

	unsigned long long Tools::GetEpoch()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	size_t Tools::GetTime(char* buffer)
	{

#if IS_OS(MEDIA_OS_WINDOWS_ALL)

		SYSTEMTIME st;
		GetLocalTime(&st);
		return _snprintf(buffer, 40, "%02d-%02d-%02d_", st.wHour, st.wMinute, st.wSecond);
#else

		timeval curTime;
		gettimeofday(&curTime, NULL);
		int pos;
		pos = strftime(buffer, 10, "%H-%M-%S_", localtime(&curTime.tv_sec));
		return pos;
#endif 
	}

	size_t Tools::GetDateTime(char* buffer)
	{
		unsigned long long epoch = GetEpoch();

#if IS_OS(MEDIA_OS_WINDOWS_ALL)

		SYSTEMTIME st;
		GetLocalTime(&st);
		return _snprintf(buffer, 40, "[%02d-%02d-%04d %02d:%02d:%02d %llu] ", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, epoch);
#else

		timeval curTime;
		gettimeofday(&curTime, NULL);
		int pos;
		pos = strftime(buffer, 22, "[%d-%m-%Y %H:%M:%S", localtime(&curTime.tv_sec));
		pos += snprintf(buffer + pos, 20, " %llu] ", epoch);

		return pos;
#endif 
	}

	std::string Tools::GetCurrentDirectoryAny()
	{
		std::string Directory;
#if defined(__ANDROID__)
		Directory = "/sdcard/";
#elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
		Directory = std::string(getenv("HOME")) + "/Documents/";
#elif defined(DESKTOP_C_SHARP)
		Directory = "";
#endif
		return Directory;
	}

	void Tools::WriteToFile(short* saDataToWriteToFile, int nLength)
	{
		if (NULL == m_filePointerToWriteShortData)
		{
			m_filePointerToWriteShortData = fopen("shortFile.pcm", "wb");
		}

		fwrite(saDataToWriteToFile, 2, nLength, m_filePointerToWriteShortData);
	}

	void Tools::WriteToFile(unsigned char* ucaDataToWriteToFile, int nLength)
	{
		if (NULL == m_filePointerToWriteByteData)
		{
			m_filePointerToWriteByteData = fopen("byteFile.test", "wb");
		}

		fwrite(ucaDataToWriteToFile, 1, nLength, m_filePointerToWriteByteData);
	}

	unsigned long long Tools::GetTotalSystemMemory()
	{
#if defined(TARGET_OS_IPHONE) || defined(__ANDROID__) || defined(TARGET_IPHONE_SIMULATOR)

		unsigned long long pages = sysconf(_SC_PHYS_PAGES);
		unsigned long long page_size = sysconf(_SC_PAGE_SIZE);

		//LOGE(" command for something <><> pages :: %llu  --  page_size  : %llu ", pages, page_size);

		return pages * page_size;

#elif defined (DESKTOP_C_SHARP)

		MEMORYSTATUSEX status;
		printf(" GetTotalSystemMemory1\n");
		status.dwLength = sizeof(status);
		printf(" GetTotalSystemMemory2\n");
		GlobalMemoryStatusEx(&status);
		printf(" GetTotalSystemMemory3\n");
		return status.ullTotalPhys;

#else 
		return 0;
#endif
	}

	unsigned long long Tools::GetAvailableSystemMemory()
	{
#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)



		long rss = 0L;
		FILE* fp = NULL;
		if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
			return (size_t)0L;      /* Can't open? */
		if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
		{
			fclose( fp );
			return (size_t)0L;      /* Can't read? */
		}
		fclose( fp );
		return (unsigned long long)(size_t)rss * (size_t)sysconf( _SC_PAGESIZE);

		//Reference: http://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-run-time-in-c#
		//Still Not tested
		//Not working

#elif defined(__ANDROID__)

		unsigned long long pages = sysconf(_SC_AVPHYS_PAGES);
		long page_size = sysconf(_SC_PAGE_SIZE);
		return pages * page_size;

#elif defined (DESKTOP_C_SHARP)

		MEMORYSTATUSEX status;
		status.dwLength = sizeof(status);
		GlobalMemoryStatusEx(&status);
		return status.ullAvailPhys;

#else 
		return 0;
#endif
	}

	void Tools::IntToUnsignedCharConversion(int number, unsigned char convertedArray[], int index)
	{
		convertedArray[index + 0] = (number >> 24) & 0xFF;
		convertedArray[index + 1] = (number >> 16) & 0xFF;
		convertedArray[index + 2] = (number >> 8) & 0xFF;
		convertedArray[index + 3] = number & 0xFF;
	}

	int Tools::UnsignedCharToIntConversion(unsigned char convertedArray[], int index)
	{
		int number;

		number = ((convertedArray[index + 0] & 0xFF) << 24) | ((convertedArray[index + 1] & 0xFF) << 16) | ((convertedArray[index + 2] & 0xFF) << 8) | convertedArray[3];

		return number;
	}

	void Tools::SetMediaUnitVersionInMediaChunck(int number, unsigned char data[])
	{
		data[LIVE_MEDIA_UNIT_VERSION_BLOCK_POSITION] = number & 0xFF;
	}

	void Tools::SetMediaUnitStreamTypeInMediaChunck(int number, unsigned char data[])
	{
		data[LIVE_MEDIA_UNIT_STREAM_TYPE_BLOCK_POSITION] = number & 0xFF;
	}

	int Tools::GetMediaUnitStreamTypeFromMediaChunck(unsigned char data[])
	{
		int number;

		number = data[LIVE_MEDIA_UNIT_STREAM_TYPE_BLOCK_POSITION] & 0xFF;

		return number;
	}

	void Tools::SetMediaUnitBlockInfoPositionInMediaChunck(int number, unsigned char data[])
	{
		data[LIVE_MEDIA_UNIT_BLOCK_INFO_POSITION_BLOCK_POSITION] = number & 0xFF;
	}

	int Tools::GetMediaUnitBlockInfoPositionFromMediaChunck(unsigned char data[])
	{
		int number;

		number = data[LIVE_MEDIA_UNIT_BLOCK_INFO_POSITION_BLOCK_POSITION] & 0xFF;

		return number;
	}

	void Tools::SetMediaUnitTimestampInMediaChunck(long long number, unsigned char data[])
	{
		SetIntegerLongLongUnsignedChar(data, LIVE_MEDIA_UNIT_TIMESTAMP_BLOCK_POSITION, LIVE_MEDIA_UNIT_TIMESTAMP_BLOCK_SIZE, number);
	}

	long long Tools::GetMediaUnitTimestampInMediaChunck(unsigned char data[])
	{
		long long  number;

		number = GetLongLongFromUnsignedChar(data, LIVE_MEDIA_UNIT_TIMESTAMP_BLOCK_POSITION, LIVE_MEDIA_UNIT_TIMESTAMP_BLOCK_SIZE);

		return number;
	}

	int Tools::GetMediaUnitVersionFromMediaChunck(unsigned char data[])
	{
		int number;

		number = data[LIVE_MEDIA_UNIT_VERSION_BLOCK_POSITION] & 0xFF;

		return number;
	}

	void Tools::SetMediaUnitHeaderLengthInMediaChunck(int number, unsigned char data[])
	{
		SetIntegerIntoUnsignedChar(data, LIVE_MEDIA_UNIT_HEADER_LENGTH_BLOCK_POSITION, LIVE_MEDIA_UNIT_HEADER_LENGTH_BLOCK_SIZE, number);
	}

	int Tools::GetMediaUnitHeaderLengthFromMediaChunck(unsigned char data[])
	{
		int number;

		number = GetIntegerFromUnsignedChar(data, LIVE_MEDIA_UNIT_HEADER_LENGTH_BLOCK_POSITION, LIVE_MEDIA_UNIT_HEADER_LENGTH_BLOCK_SIZE);

		return number;
	}

	void Tools::SetMediaUnitChunkDurationInMediaChunck(int number, unsigned char data[])
	{
		SetIntegerIntoUnsignedChar(data, LIVE_MEDIA_UNIT_CHUNK_DURATION_BLOCK_POSITION, LIVE_MEDIA_UNIT_CHUNK_DURATION_BLOCK_SIZE, number);
	}

	int Tools::GetMediaUnitChunkDurationFromMediaChunck(unsigned char data[])
	{
		int number;

		number = GetIntegerFromUnsignedChar(data, LIVE_MEDIA_UNIT_CHUNK_DURATION_BLOCK_POSITION, LIVE_MEDIA_UNIT_CHUNK_DURATION_BLOCK_SIZE);

		return number;
	}

	void Tools::SetMediaUnitChunkNumberInMediaChunck(int number, unsigned char data[])
	{
		SetIntegerIntoUnsignedChar(data, LIVE_MEDIA_UNIT_CHUNK_NUMBER_BLOCK_POSITION, LIVE_MEDIA_UNIT_CHUNK_NUMBER_BLOCK_SIZE, number);
	}

	int Tools::GetMediaUnitChunkNumberFromMediaChunck(unsigned char data[])
	{
		int number;

		number = GetIntegerFromUnsignedChar(data, LIVE_MEDIA_UNIT_CHUNK_NUMBER_BLOCK_POSITION, LIVE_MEDIA_UNIT_CHUNK_NUMBER_BLOCK_SIZE);

		return number;
	}

	void Tools::SetAudioBlockSizeInMediaChunck(int number, unsigned char data[])
	{
		SetIntegerIntoUnsignedChar(data, LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_POSITION, LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_SIZE, number);
	}

	int Tools::GetAudioBlockSizeFromMediaChunck(unsigned char data[])
	{
		int number;

		number = GetIntegerFromUnsignedChar(data, LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_POSITION, LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_SIZE);

		return number;
	}




	void Tools::SetAudioBlockStartingPositionInMediaChunck(int number, unsigned char data[])
	{
		SetIntegerIntoUnsignedChar(data, LIVE_MEDIA_UNIT_AUDIO_STARTING_POSITION_BLOCK_POSITION, LIVE_MEDIA_UNIT_AUDIO_STARTING_POSITION_BLOCK_SIZE, number);
	}

	int Tools::GetAudioBlockStartingPositionFromMediaChunck(unsigned char data[])
	{
		int number;

		number = GetIntegerFromUnsignedChar(data, LIVE_MEDIA_UNIT_AUDIO_STARTING_POSITION_BLOCK_POSITION, LIVE_MEDIA_UNIT_AUDIO_STARTING_POSITION_BLOCK_SIZE);

		return number;
	}

	void Tools::SetVideoBlockStartingPositionInMediaChunck(int number, unsigned char data[])
	{
		SetIntegerIntoUnsignedChar(data, LIVE_MEDIA_UNIT_VIDEO_STARTING_POSITION_BLOCK_POSITION, LIVE_MEDIA_UNIT_VIDEO_STARTING_POSITION_BLOCK_SIZE, number);
	}

	int Tools::GetVideoBlockStartingPositionFromMediaChunck(unsigned char data[])
	{
		int number;

		number = GetIntegerFromUnsignedChar(data, LIVE_MEDIA_UNIT_VIDEO_STARTING_POSITION_BLOCK_POSITION, LIVE_MEDIA_UNIT_VIDEO_STARTING_POSITION_BLOCK_SIZE);

		return number;
	}



	void Tools::SetVideoBlockSizeInMediaChunck(int number, unsigned char data[])
	{
		SetIntegerIntoUnsignedChar(data, LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_POSITION, LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_SIZE, number);
	}

	int Tools::GetVideoBlockSizeFromMediaChunck(unsigned char data[])
	{
		int number;

		number = GetIntegerFromUnsignedChar(data, LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_POSITION, LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_SIZE);

		return number;
	}

	void Tools::SetNumberOfAudioFramesInMediaChunck(int index, int number, unsigned char data[])
	{
		SetIntegerIntoUnsignedChar(data, index, LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_FRAME_BLOCK_SIZE, number);
	}

	int Tools::GetNumberOfAudioFramesFromMediaChunck(int index, unsigned char data[])
	{
		int number;

		number = GetIntegerFromUnsignedChar(data, index, LIVE_MEDIA_UNIT_NUMBER_OF_AUDIO_FRAME_BLOCK_SIZE);

		return number;
	}

	void Tools::SetNumberOfVideoFramesInMediaChunck(int index, int number, unsigned char data[])
	{
		SetIntegerIntoUnsignedChar(data, index, LIVE_MEDIA_UNIT_NUMBER_OF_VIDEO_FRAME_BLOCK_SIZE, number);
	}

	int Tools::GetNumberOfVideoFramesFromMediaChunck(int index, unsigned char data[])
	{
		int number;

		number = GetIntegerFromUnsignedChar(data, index, LIVE_MEDIA_UNIT_NUMBER_OF_VIDEO_FRAME_BLOCK_SIZE);

		return number;
	}

	void Tools::SetServiceTypeInMediaChunck(int number, unsigned char data[])
	{
		SetIntegerIntoUnsignedChar(data, LIVE_MEDIA_UNIT_SERVICE_TYPE_BLOCK_POSITION, LIVE_MEDIA_UNIT_SERVICE_TYPE_BLOCK_SIZE, number);
	}

	int Tools::GetServiceTypeFromMediaChunck(unsigned char data[])
	{
		int number;

		number = GetIntegerFromUnsignedChar(data, LIVE_MEDIA_UNIT_SERVICE_TYPE_BLOCK_POSITION, LIVE_MEDIA_UNIT_SERVICE_TYPE_BLOCK_SIZE);

		return number;
	}

	void Tools::SetEntityTypeInMediaChunck(int number, unsigned char data[])
	{
		SetIntegerIntoUnsignedChar(data, LIVE_MEDIA_UNIT_ENTITY_TYPE_BLOCK_POSITION, LIVE_MEDIA_UNIT_ENTITY_TYPE_BLOCK_SIZE, number);
	}

	int Tools::GetEntityTypeFromMediaChunck(unsigned char data[])
	{
		int number;

		number = GetIntegerFromUnsignedChar(data, LIVE_MEDIA_UNIT_ENTITY_TYPE_BLOCK_POSITION, LIVE_MEDIA_UNIT_ENTITY_TYPE_BLOCK_SIZE);

		return number;
	}

	void Tools::SetNextAudioFramePositionInMediaChunck(int index, int number, unsigned char data[])
	{
		SetIntegerIntoUnsignedChar(data, index, LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_SIZE, number);
	}

	int Tools::GetNextAudioFramePositionFromMediaChunck(int index, unsigned char data[])
	{
		int number;

		number = GetIntegerFromUnsignedChar(data, index, LIVE_MEDIA_UNIT_AUDIO_SIZE_BLOCK_SIZE);

		return number;
	}

	void Tools::SetNextVideoFramePositionInMediaChunck(int index, int number, unsigned char data[])
	{
		SetIntegerIntoUnsignedChar(data, index, LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_SIZE, number);
	}

	int Tools::GetNextVideoFramePositionFromMediaChunck(int index, unsigned char data[])
	{
		int number;

		number = GetIntegerFromUnsignedChar(data, index, LIVE_MEDIA_UNIT_VIDEO_SIZE_BLOCK_SIZE);

		return number;
	}


	int Tools::GetIntegerFromUnsignedChar(unsigned char *packetData, int index, int nLenght)
	{
		int result = 0;
		int interval = 8;
		int startPoint = (nLenght - 1) << 3;

		for (int i = startPoint; i >= 0; i -= interval)
		{
			result += (packetData[index++] & 0xFF) << i;
		}

		return result;
	}

	void Tools::SetIntegerIntoUnsignedChar(unsigned char *packetData, int index, int nLenght, int value)
	{
		int interval = 8;

		for (int i = 0; i < nLenght; i++)
		{
			packetData[index + i] = (value >> (interval*(nLenght - i - 1)) & 0xFF);
		}
	}

	long long Tools::GetLongLongFromUnsignedChar(unsigned char *packetData, int index, int nLenght)
	{
		long long result = 0;
		int interval = 8;
		int startPoint = (nLenght - 1) << 3;

		for (int i = startPoint; i >= 0; i -= interval)
		{
			long long tempValue = (packetData[index++] & 0xFF);
			result += tempValue << i;
		}

		return result;
	}

	void Tools::SetIntegerLongLongUnsignedChar(unsigned char *packetData, int index, int nLenght, long long value)
	{
		int interval = 8;

		for (int i = 0; i < nLenght; i++)
		{
			packetData[index + i] = (value >> (interval*(nLenght - i - 1)) & 0xFF);
		}
	}


	int Tools::GetEncodedFrameType(unsigned char *pFrame)
	{
		int nalType = pFrame[2] == 1 ? (pFrame[3] & 0x1f) : (pFrame[4] & 0x1f);
		//CLogPrinter::Log("nalType = %d", nalType);
		//nalType = 7 [SPS Found]
		//nalType = 8 [PPS Found]
		//nalType = 5 [IDR Slice Found]
		//nalType = 1 [Non IDR Slice Found]

		return nalType;
	}

	void Tools::SetThreadName(const char* threadName)
	{
#if defined(__ANDROID__)
		//prctl Maximum ThreadName must be withing 16 bytes,
		//15 bytes char and 16th bytes with EOF

		prctl(PR_SET_NAME, threadName, 0, 0, 0);

#elif defined(DESKTOP_C_SHARP) || defined(TARGET_OS_WINDOWS_PHONE)
		
		uint32_t dwThreadID = GetCurrentThreadId();
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = threadName;
		info.dwThreadID = dwThreadID;
		info.dwFlags = 0;

		__try
		{
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{

		}
		
#else
		//do nothing
#endif
	}


} //namespace MediaSDK


