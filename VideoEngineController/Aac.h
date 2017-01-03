#ifndef ACC_H
#define ACC_H

#if defined(TARGET_OS_WINDOWS_PHONE)
#include <windows.h>
#endif

#include <string>
#include <queue>
#include <thread>
#include "LogPrinter.h"
#include "Tools.h"
#include "aacdecoder_lib.h"
//#include "CommonElementsBucket.h"

static string acc_tag = "AAC_LOG:";
#define AAC_LOG(a) CLogPrinter_WriteSpecific6(CLogPrinter::INFO, acc_tag + a);

#define HEADER_SIZE 512
#define AUDIO_PACKET_TYPE 7
#define VIDEO_PACKET_TYPE 1

#define MAX_NUM_OF_FRAMES 20
#define MAX_BUF_SIZE 10000

//#define DUMP_OUTPUT


class CCommonElementsBucket;


struct DecodedFrame
{
	int frameSize;
	int sampleRate;
	int numOfChannels;
	short* frame;
};


class CAac
{
private:
	FILE *m_fd;
	bool m_bIsRunningUDPDataSendingThread;
	bool m_bIsClosedUDPDataSendingThread;

	unsigned char m_sBuffer[1024];
	unsigned char m_sFrames[MAX_NUM_OF_FRAMES][MAX_BUF_SIZE];
	int m_nAudioFrameSizes[MAX_NUM_OF_FRAMES];
	int m_nVideoFrameSizes[MAX_NUM_OF_FRAMES];

	int m_nRC;
	short m_nOutBuffer[MAX_BUF_SIZE];

	struct DecodedFrame m_stDecodeFrameIn;
	struct DecodedFrame m_stDecodedFrameOut;
	std::queue<struct DecodedFrame> m_qProcessedPcmData;

	HANDLE_AACDECODER m_hDecoder;
	Tools m_cTools;
//	CCommonElementsBucket *m_cCommonElementsBucket;

	
protected:
	void CreateConfBuf(int sampleRate, int numOfChannels, unsigned char *conf);
	int ReadFileIntoFrames(std::string fileName);

	short ByteArrayToShortBE(unsigned char *byteArray);
	int ThreeBytesIntoIntBE(unsigned char *byteArray);

	void SendProcessedData();
	std::thread StartDataSendingThread();
	void StopDataSendingThread();


public:
	CAac();
	~CAac();

	bool SetParameters(int sampleRate, int numberOfChannels);
	bool DecodeFrame(unsigned char *inputDataBuffer, int inputDataSize, short *outputDataBuffer, int &dataSize);
	bool DecodeFile();
};


#endif
