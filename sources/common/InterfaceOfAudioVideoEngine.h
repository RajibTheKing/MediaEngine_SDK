
#ifndef IPV_INTERFACE_OF_AUDIO_VIDEO_ENGINEE_H
#define IPV_INTERFACE_OF_AUDIO_VIDEO_ENGINEE_H

#include <vector>
#include <string>

#define CALL_IN_LIVE_TYPE_AUDIO_ONLY 1
#define CALL_IN_LIVE_TYPE_VIDEO_ONLY 2
#define CALL_IN_LIVE_TYPE_AUDIO_VIDEO 3

#define LIVE_CALL_SCREEN_SPLIT_TYPE_DIVIDE 1
#define LIVE_CALL_SCREEN_SPLIT_TYPE_SUBSET 0

#define MEDIA_TYPE_AUDIO 1
#define MEDIA_TYPE_VIDEO 2
#define MEDIA_TYPE_LIVE_STREAM 3
#define MEDIA_TYPE_LIVE_CALL_AUDIO 4
#define MEDIA_TYPE_LIVE_CALL_VIDEO 5

#define CAMARA_VIDEO_DATA 51
#define SCREEN_VIDEO_DATA 52

#define SERVICE_TYPE_CALL 11
#define SERVICE_TYPE_SELF_CALL 13

#define ENTITY_TYPE_CALLER 31

#define SERVICE_TYPE_LIVE_STREAM 12
#define SERVICE_TYPE_SELF_STREAM 14

#define SERVICE_TYPE_CHANNEL 16

#define CHANNEL_TYPE_AUDIO 1
#define CHANNEL_TYPE_VIDEO 2
#define CHANNEL_TYPE_TV 3
#define CHANNEL_TYPE_NOT_CHANNEL 0

#define VIDEO_QUALITY_HIGH 3
#define VIDEO_QUALITY_MEDIUM 2
#define VIDEO_QUALITY_LOW 1
#define VIDEO_QUALITY_MUCH_LOW 0

#define ENTITY_TYPE_PUBLISHER 31
#define ENTITY_TYPE_VIEWER 32
#define ENTITY_TYPE_VIEWER_CALLEE 2
#define ENTITY_TYPE_PUBLISHER_CALLER 1

#define AUDIO_PLAYER_DEFAULT 1
#define AUDIO_PLAYER_HEADPHONE 2
#define AUDIO_PLAYER_LOUDSPEAKER 3

#define AUDIO_EVENT_PEER_TOLD_TO_STOP_VIDEO 1
#define AUDIO_EVENT_I_TOLD_TO_STOP_VIDEO 2
#define AUDIO_EVENT_FIRE_ENCODING_TIME 3
#define AUDIO_EVENT_FIRE_AVG_ENCODING_TIME 4
#define AUDIO_EVENT_TRACE_NOTIFICATION 5
#define AUDIO_EVENT_MEDIA_AEC_STATUS 6

#define SESSION_ID_FOR_SELF_VIEW -1000

typedef long long IPVLongType;
typedef long long LongLong;



#define DECODED_MACRO_FRAME_SIZE_FOR_MULTI (480 * 640 * 3)/2 + 1

//#define NO_CONNECTIVITY

namespace MediaSDK
{

	enum AudioCodecTypeEnum
	{
		AUDIO_CODEC_UNKNOWN = -1,
		AUDIO_CODEC_PCM = 1,
		AUDIO_CODEC_OPUS = 2,
		AUDIO_CODEC_AAC = 3
	};

	struct AudioCallParams
	{
		int nAudioSpeakerType;
		int nTraceInfoLength;
		int * npTraceInfo;
		bool bDeviceHasAEC;
		std::string sManuName;
		std::string sModelName;
		std::string sOSVersion;
		int nSDKVersion;
		AudioCodecTypeEnum nAudioCodecType;
		int nAudioServiceType;

	private: 
		int a_npTraceInfo[3];
	public:
		AudioCallParams()
		{
			a_npTraceInfo[0] = a_npTraceInfo[1] = a_npTraceInfo[2] = 0;
			bDeviceHasAEC = true;
			nAudioSpeakerType = AUDIO_PLAYER_DEFAULT;
			nTraceInfoLength = 3;
			npTraceInfo = a_npTraceInfo;
			sManuName = "";
			sModelName = "";
			sOSVersion = "";
			nSDKVersion = 0;
			nAudioCodecType = AUDIO_CODEC_UNKNOWN;
			nAudioServiceType = SERVICE_TYPE_CALL;
		}
	};

	class CInterfaceOfAudioVideoEngine
	{

	public:

		CInterfaceOfAudioVideoEngine();
		CInterfaceOfAudioVideoEngine(const char* szLoggerPath, int nLoggerPrintLevel);
		~CInterfaceOfAudioVideoEngine();

		bool Init(const IPVLongType& llUserID, const char* szLoggerPath, int nLoggerPrintLevel);
		bool InitializeLibrary(const IPVLongType& llUserID);
		bool SetUserName(const IPVLongType llUserName);
		bool StartAudioCall(const IPVLongType llFriendID, int nServiceType, int nEntityType, AudioCallParams acParams);

		bool SetVolume(const LongLong lFriendID, int iVolume, bool bRecorder);
		bool SetSpeakerType(const LongLong lFriendID, AudioCallParams acParams);

		void NotifyCameraMode(const LongLong lFriendID, bool bCameraEnable);
		void NotifyMicrophoneMode(const LongLong lFriendID, bool bMicrophoneEnable);

		bool StartCallInLive(const IPVLongType llFriendID, int iRole, int nCallInLiveType, int nScreenSplitType, AudioCallParams acParams);
		bool EndCallInLive(const IPVLongType llFriendID);

		void SetCallInLiveType(const IPVLongType llFriendID, int nCallInLiveType);

		bool StartLiveStreaming(const IPVLongType llFriendID, int nEntityType, bool bAudioOnlyLive, int nVideoHeight, int nVideoWidth, AudioCallParams acParams);
		bool StartChannel(const IPVLongType llFriendID, int nChannelType, int nEntityType, bool bAudioOnlyLive, int nVideoHeight, int nVideoWidth, AudioCallParams acParams);

		bool StartVideoCall(const IPVLongType llFriendID, int nVideoHeight, int nVideoWidth, int nServiceType, int nEntityType, int nNetworkType, bool bAudioOnlyLive);
		int EncodeAndTransfer(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength);
		int PushPacketForDecoding(const IPVLongType llFriendID, int mediaType, int nEntityType, unsigned char *in_data, unsigned int unLength);
		int PushAudioForDecodingVector(const IPVLongType llFriendID, int mediaType, int nEntityType, unsigned char *in_data, unsigned int unLength, std::vector< std::pair<int, int> > vMissingFrames);
		int SendAudioData(const IPVLongType llFriendID, short *in_data, unsigned int unLength);
		int SendVideoData(const IPVLongType llFriendID, unsigned char *in_data, unsigned int unLength, unsigned int nOrientationType, int device_orientation);
		int SetEncoderHeightWidth(const IPVLongType llFriendID, int nVideoHeight, int nVideoWidth, int nDataType = CAMARA_VIDEO_DATA);
		int SetDeviceDisplayHeightWidth(int nVideoHeight, int nVideoWidth);

		int SetVideoQualityForLive(const IPVLongType llFriendID, int quality);

        int SetBeautification(const IPVLongType llFriendID, bool bIsEnable);
		int SetVideoEffect(const IPVLongType llFriendID, int nEffectStatus);
		int TestVideoEffect(const IPVLongType llFriendID, int *param, int size);

		int SetBitRate(const IPVLongType llFriendID, int nBitRate);

		int CheckDeviceCapability(const LongLong& lFriendID, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow);
		int SetDeviceCapabilityResults(int iNotification, int iHeightHigh, int iWidthHigh, int iHeightLow, int iWidthLow);

		bool StopAudioCall(const IPVLongType llFriendID);
		bool StopVideoCall(const IPVLongType llFriendID);
		void SetLoggerPath(std::string strLoggerPath);
		bool SetLoggingState(bool bLoggingState, int nLogLevel);
		void UninitializeLibrary();

		int StartAudioEncodeDecodeSession();
		int EncodeAudioFrame(short *psaEncodingDataBuffer, int nAudioFrameSize, unsigned char *ucaEncodedDataBuffer);
		int DecodeAudioFrame(unsigned char *ucaDecodedDataBuffer, int nAudioFrameSize, short *psaDecodingDataBuffer);
		int StopAudioEncodeDecodeSession();

		void SetNotifyClientWithPacketCallback(void(*callBackFunctionPointer)(long long, unsigned char*, int));
		void SetNotifyClientWithVideoDataCallback(void(*callBackFunctionPointer)(long long, int, unsigned char*, int, int, int, int, int, int));

		void SetNotifyClientWithMultVideoDataCallback(void(*callBackFunctionPointer)(unsigned char[][DECODED_MACRO_FRAME_SIZE_FOR_MULTI], int*, int*, int*, int));

		void SetNotifyClientWithVideoNotificationCallback(void(*callBackFunctionPointer)(long long, int));
		void SetNotifyClientWithNetworkStrengthNotificationCallback(void(*callBackFunctionPointer)(IPVLongType, int));
		void SetNotifyClientWithAudioDataCallback(void(*callBackFunctionPointer)(long long, int, short*, int));
		void SetNotifyClientWithAudioPacketDataCallback(void(*callBackFunctionPointer)(IPVLongType, unsigned char*, int));
		void SetNotifyClientWithAudioAlarmCallback(void(*callBackFunctionPointer)(long long, int*, int));

		void SetSendFunctionPointer(void(*callBackFunctionPointer)(IPVLongType, int, unsigned char*, int, int, std::vector< std::pair<int, int> > vAudioBlocks));

		int StartVideoMuxingAndEncodeSession(unsigned char *pBMP32Data, int iLen, int nVideoHeight, int nVideoWidth);
		int FrameMuxAndEncode(unsigned char *pVideoYuv, int iHeight, int iWidth);
		int StopVideoMuxingAndEncodeSession(unsigned char *finalData);

		int StartMultiResolutionVideoSession(int *targetHeight, int *targetWidth, int iLen);
		int MakeMultiResolutionVideo( unsigned char *pVideoYuv, int iLen );
		int StopMultiResolutionVideoSession();

		void InterruptOccured(const LongLong lFriendID);
		void InterruptOver(const LongLong lFriendID);

		std::string GetMediaEngineVersion();

	private:

		long long m_llTimeOffset;
	};

} //namespace MediaSDK

#endif
