#include "Aac.h"
#include <queue>
#include <thread>


CAac::CAac() :
m_bIsClosedUDPDataSendingThread(true),
m_hDecoder(nullptr)
{
	AAC_LOG("Initialize ACC decoder.");

#ifdef DUMP_OUTPUT
	std::string outFilePath = "/sdcard/naac_file/output_20s.pcm";
	m_fd = fopen(outputFilePath.c_str(), "wb");

	if (!m_fd){
		AAC_LOG("Couldn't create file to write.");
	}
#endif // DUMP_OUTPUT

	TRANSPORT_TYPE transportType = TT_MP4_RAW;
	m_hDecoder = aacDecoder_Open(transportType, 1);

	if (m_hDecoder == nullptr) {
		AAC_LOG("Unable to open decoder.");
	}

//	std::thread udpDataRecvThread = StartDataSendingThread();
//	udpDataRecvThread.detach();
}


CAac::~CAac()
{
	AAC_LOG("De-allocate AAC decoder.");

	if (m_hDecoder != nullptr) {
		aacDecoder_Close(m_hDecoder);
	}

#ifdef DUMP_OUTPUT
	if (m_fd) {
		fclose(m_fd);
		m_fd = nullptr;
	}
#endif // DUMP_OUTPUT

	StopDataSendingThread();
}


void CAac::CreateConfBuf(int sampleRate, int numOfChannels, unsigned char *conf)
{
	short sampleMode = 0;

	switch (sampleRate)
	{
	case 96000:
		sampleMode = 0;
		break;

	case 88200:
		sampleMode = 1;
		break;

	case 64000:
		sampleMode = 2;
		break;

	case 48000:
		sampleMode = 3;
		break;

	case 44100:
		sampleMode = 4;
		break;

	case 32000:
		sampleMode = 5;
		break;

	case 24000:
		sampleMode = 6;
		break;

	case 22050:
		sampleMode = 7;
		break;

	case 16000:
		sampleMode = 8;
		break;

	case 12000:
		sampleMode = 9;
		break;

	case 11025:
		sampleMode = 10;
		break;

	case 8000:
		sampleMode = 11;
		break;

	case 7350:
		sampleMode = 12;
		break;

	default:
		AAC_LOG("sampleRate not matched!!!");
		break;
	}

	short channelMode = numOfChannels;
	if (numOfChannels == 8)	{
		channelMode = 7;
	}

	short config = 0;
	config |= 2 << (sizeof(short) * 8 - 5);                                           // 5 bits object type , 2 == AAC LC
	config |= sampleMode << (sizeof(short) * 8 - 5 - 4);                              // 4 bits frequency type
	config |= channelMode << (sizeof(short) * 8 - 5 - 4 - 4);                         // 4 bits channel configuration

	AAC_LOG("sampleMode: " + m_cTools.IntegertoStringConvert(sampleMode) + ", channelMode: " + m_cTools.IntegertoStringConvert(channelMode) + ", config: " + m_cTools.IntegertoStringConvert(config));

	conf[0] = config >> 8;
	conf[1] = config;
}


bool CAac::SetParameters(int sampleRate, int numberOfChannels)
{
	AAC_LOG("CAcc::SetParameters(), sampleRate: " + m_cTools.IntegertoStringConvert(sampleRate) + ", numberOfChannels: " + m_cTools.IntegertoStringConvert(numberOfChannels));

	if (aacDecoder_SetParam(m_hDecoder, AAC_PCM_OUTPUT_INTERLEAVED, 1))
	{
		AAC_LOG("Couldn't set AAC_PCM_OUTPUT_INTERLEAVED");
		return false;
	}

	if (aacDecoder_SetParam(m_hDecoder, AAC_PCM_MIN_OUTPUT_CHANNELS, numberOfChannels))
	{
		AAC_LOG("Couldn't set AAC_PCM_MIN_OUTPUT_CHANNELS");
		return false;
	}

	if (aacDecoder_SetParam(m_hDecoder, AAC_PCM_MAX_OUTPUT_CHANNELS, numberOfChannels))
	{
		AAC_LOG("Couldn't set AAC_PCM_MAX_OUTPUT_CHANNELS");
		return false;
	}

	if (aacDecoder_SetParam(m_hDecoder, AAC_PCM_OUTPUT_CHANNEL_MAPPING, 1))
	{
		AAC_LOG("Couldn't set AAC_PCM_OUTPUT_CHANNEL_MAPPING");
		return false;
	}

	const unsigned int confSize = 2;
	unsigned char conf[confSize];
	unsigned char *conf2 = conf;

	CreateConfBuf(sampleRate, numberOfChannels, conf);

	if (aacDecoder_ConfigRaw(m_hDecoder, &conf2, &confSize))
	{
		AAC_LOG("Couldn't set aacDecoder_ConfigRaw.");
		return false;
	}

	AAC_LOG("Set all settings successfully.");

	return true;
}


bool CAac::DecodeFrame(unsigned char *inputDataBuffer, int inputDataSize, short *outputDataBuffer, int &dataSize)
{
	AAC_LOG("CAcc::DecodeFrame(), inputDataSize: " + m_cTools.IntegertoStringConvert(inputDataSize));

	m_nRC = aacDecoder_Fill(m_hDecoder, &inputDataBuffer, (const unsigned int *)&inputDataSize, (unsigned int *)&inputDataSize);
	if (m_nRC != AAC_DEC_OK)
	{
		AAC_LOG("aacDecoder_Fill error code: " + m_cTools.IntegertoStringConvert(m_nRC));
		return false;
	}

	m_nRC = aacDecoder_DecodeFrame(m_hDecoder, m_nOutBuffer, MAX_BUF_SIZE, 0);
	if (m_nRC != AAC_DEC_OK)
	{
		AAC_LOG("aacDecoder_GetStreamInfo with error code: " + m_cTools.IntegertoStringConvert(m_nRC));
		return false;
	}

	CStreamInfo* sInfo = aacDecoder_GetStreamInfo(m_hDecoder);

	m_stDecodeFrameIn.frameSize = sInfo->frameSize;
	m_stDecodeFrameIn.sampleRate = sInfo->sampleRate;
	m_stDecodeFrameIn.numOfChannels = sInfo->numChannels;

//	m_stDecodeFrameIn.frame = new short[m_stDecodeFrameIn.frameSize * m_stDecodeFrameIn.numOfChannels];
//	memcpy(m_stDecodeFrameIn.frame, m_nOutBuffer, (m_stDecodeFrameIn.frameSize * m_stDecodeFrameIn.numOfChannels * 2));

//	m_qProcessedPcmData.push(m_stDecodeFrameIn);

	AAC_LOG("SampleRate: " + m_cTools.IntegertoStringConvert(m_stDecodeFrameIn.sampleRate));
	dataSize = m_stDecodeFrameIn.frameSize * m_stDecodeFrameIn.numOfChannels;
	memcpy(outputDataBuffer, m_nOutBuffer, (dataSize * 2));

	return true;
}


void CAac::SendProcessedData()
{
	AAC_LOG("CAac::SendProcessedData()");

	int i = 0;

	m_bIsRunningUDPDataSendingThread = true;
	m_bIsClosedUDPDataSendingThread = false;

	while (m_bIsRunningUDPDataSendingThread)
	{
		auto t_start = std::chrono::high_resolution_clock::now();

		if (!m_qProcessedPcmData.empty())
		{
			m_stDecodedFrameOut = m_qProcessedPcmData.front();

			AAC_LOG(m_cTools.IntegertoStringConvert(i++) + ".SampleRate: " + m_cTools.IntegertoStringConvert(m_stDecodedFrameOut.sampleRate) + " frameSize: " + m_cTools.IntegertoStringConvert(m_stDecodedFrameOut.frameSize) + " numOfChannels: " + m_cTools.IntegertoStringConvert(m_stDecodedFrameOut.numOfChannels));
			//m_cCommonElementsBucket->m_pEventNotifier->fireAudioEvent(5, SERVICE_TYPE_CALL, (m_stDecodedFrameOut.frameSize * m_stDecodedFrameOut.numOfChannels), m_stDecodedFrameOut.frame);

#ifdef DUMP_OUTPUT
			fwrite(m_stDecodedFrameOut.frame, 2, (m_stDecodedFrameOut.frameSize * m_stDecodedFrameOut.numOfChannels), m_fd);
#endif // DUMP_OUTPUT

			m_qProcessedPcmData.pop();
			delete[] m_stDecodedFrameOut.frame;

			auto t_end = std::chrono::high_resolution_clock::now();
			auto t_diff = std::chrono::duration<double, std::milli>(t_end - t_start).count() + 1.5;
			m_cTools.SOSleep((m_stDecodedFrameOut.sampleRate / (m_stDecodedFrameOut.frameSize * m_stDecodedFrameOut.numOfChannels)) - t_diff);
		}
		else
		{
			m_cTools.SOSleep(10);
		}
	}

	m_bIsClosedUDPDataSendingThread = true;
}


std::thread CAac::StartDataSendingThread()
{
	return std::thread([=] { SendProcessedData(); });
}


void CAac::StopDataSendingThread()
{
	AAC_LOG("CAac::StopDataSendingThread()");

	m_bIsRunningUDPDataSendingThread = false;
	while (!m_bIsClosedUDPDataSendingThread){
		m_cTools.SOSleep(10);
	}
}


short CAac::ByteArrayToShortBE(unsigned char *byteArray)
{
	short num;

	num = num & 0x0000;
	num = num | (byteArray[0] << 8);
	num = num | byteArray[1];

	return num;
}


int CAac::ThreeBytesIntoIntBE(unsigned char *byteArray){
	return (byteArray[2] & 0xFF) | ((byteArray[1] & 0xFF) << 8) | ((byteArray[0] & 0xFF) << 16);
}


int CAac::ReadFileIntoFrames(std::string fileName)
{
	int totFileSize = -1;
	FILE *fd = fopen(fileName.c_str(), "rb");

	if (!fd){
		AAC_LOG("file open failed");
		return false;
	}

	if (!fseek(fd, 0, SEEK_END))
	{
		totFileSize = ftell(fd);
	}

	fseek(fd, 0, SEEK_SET);
	fread(m_sBuffer, 1, 12, fd);

	int version = int(m_sBuffer[0]);
	int audioDataSize = ThreeBytesIntoIntBE(&m_sBuffer[5]);
	int videoDataSize = ThreeBytesIntoIntBE(&m_sBuffer[8]);
	int numOfAudioFrames = int(m_sBuffer[11]);
	AAC_LOG("NumberOfAudioFrames: " + m_cTools.IntegertoStringConvert(numOfAudioFrames));

	fread(m_sBuffer, 1, (3 * numOfAudioFrames) + 1, fd);
	for (int i = 0; i < numOfAudioFrames; i++) 
	{
		m_nAudioFrameSizes[i] = ThreeBytesIntoIntBE(&m_sBuffer[i * 3]);
	}

	int numOfVideoFrames = int(m_sBuffer[3 * numOfAudioFrames]);
//	AAC_LOG("NumOfVideoFrames: " + m_cTools.IntegertoStringConvert(numOfVideoFrames));

	fread(m_sBuffer, 1, (3 * numOfVideoFrames), fd);
	for (int i = 0; i < numOfVideoFrames; i++) 
	{
		m_nVideoFrameSizes[i] = ThreeBytesIntoIntBE(&m_sBuffer[i * 3]);
	}

	int frameStartPos = (HEADER_SIZE * 3);
	fseek(fd, frameStartPos, 0);

	int audioFrameCnt, videoFrameCnt;
	audioFrameCnt = videoFrameCnt = 0;

	for (int i = 0; i < (numOfAudioFrames + numOfVideoFrames); i++)
	{
		fread(m_sBuffer, 1, 12, fd);

		if (AUDIO_PACKET_TYPE == int(m_sBuffer[0]))
		{
			fread(m_sFrames[audioFrameCnt], 1, m_nAudioFrameSizes[audioFrameCnt], fd);
//			AAC_LOG(m_cTools.IntegertoStringConvert(audioFrameCnt) + ". audio: " + m_cTools.IntegertoStringConvert(m_nAudioFrameSizes[audioFrameCnt]));
			audioFrameCnt++;
		}
		else
		{
			fseek(fd, m_nVideoFrameSizes[videoFrameCnt] + 4, SEEK_CUR);
//			AAC_LOG(m_cTools.IntegertoStringConvert(videoFrameCnt) + ". video: " + m_cTools.IntegertoStringConvert(m_nVideoFrameSizes[videoFrameCnt]));
			videoFrameCnt++;
		}
	}

	fclose(fd);

	return numOfAudioFrames;

}


bool CAac::DecodeFile()
{
	auto decodeAndPlay = [&](std::string inputFilePath)
	{
		AAC_LOG("DecodingFile: " + inputFilePath);
		int len;
		int numberOfFrames = ReadFileIntoFrames(inputFilePath.c_str());                   // outputaudio_h264_2.naac ;   input_file.naac

		for (int i = 0; i < numberOfFrames; i++)
		{
			DecodeFrame(m_sFrames[i], m_nAudioFrameSizes[i], m_nOutBuffer, len);
		}
	};

	auto playFromFile = [&]()
	{
		int totFileSize = -1;
		int frameSize = 1024;
		short outBuffer[10000];

		if (!m_fd){
			AAC_LOG("file open failed");
			return false;
		}

		if (!fseek(m_fd, 0, SEEK_END))
		{
			totFileSize = ftell(m_fd);
		}

		fseek(m_fd, 0, SEEK_SET);
		struct DecodedFrame decodedFrame;

		while (totFileSize > 1500)
		{
			fread(outBuffer, 2, frameSize*2, m_fd);

			decodedFrame.frameSize = frameSize;
			decodedFrame.numOfChannels = 2;
			decodedFrame.sampleRate = 48000;

			decodedFrame.frame = new short[frameSize*2];
			memcpy(decodedFrame.frame, outBuffer, (frameSize * 2 * 2));

			m_qProcessedPcmData.push(decodedFrame);
			totFileSize = totFileSize - (frameSize*2*2);
		}
        return true;
	};

	std::string inputFilePath = "D:\\AudioTeam\\AAC\\fdk-aac-master\\aacTest\\TestFile\\10Min Data\\chunks\\chunk.";
	for (int i = 1; i <= 100; i++) {
		decodeAndPlay(inputFilePath + m_cTools.IntegertoStringConvert(i));
	}
//	playFromFile();

	return true;
}

