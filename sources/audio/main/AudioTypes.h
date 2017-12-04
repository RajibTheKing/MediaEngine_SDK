#ifndef AUDIO_TYPES_H
#define AUDIO_TYPES_H

#include <vector>
#include <cstddef>
#include <climits>
#include <unordered_map>
#include <string>

namespace MediaSDK
{
	//External Callbacks
	typedef void(*SendFunctionPointerType)(long long, int, unsigned char*, int, int, std::vector< std::pair<int, int> >);

	//Internal Callback Interfaces.
	//TODO: Should go on different header
	class DataReadyListenerInterface
	{
	public:
		virtual void OnDataReadyToSend(int mediaType, unsigned char* data, size_t dataLength) = 0;
	};

	class PacketEventListener
	{
	public:
		virtual void FirePacketEvent(int eventType, size_t dataLenth, unsigned char data[]) = 0;
	};
	
	class DataEventListener
	{
	public:
		virtual void FireDataEvent(int eventType, size_t dataLenth, short data[]) = 0;
	};
	
	class NetworkChangeListener
	{
	public:
		virtual void FireNetworkChange(int eventType) = 0;
	};
	
	class AudioAlarmListener
	{
	public: 
		virtual void FireAudioAlarm(int eventType) = 0;
	};

	class PackatizedDataListener
	{
	public:
		virtual void SendPackatizedData(unsigned char*, int) = 0;
	};

	class DepackatizedDataListener
	{
	public:
		virtual void SendDepackatizedData(unsigned char*, int) = 0;
	};

	class DeviceInformationInterface
	{
	public:
		virtual void SetDeviceInformationOfAnotherRole(unsigned char *ucaInfo, int len) = 0;
	};

	enum AudioEntityRoleType
	{
		EntityInCall,
		EntityChannel,

		EntityPublisher,
		EntityPublisherInCall,

		EntityViewer,
		EntityViewerInCall,

		EntityNone
	};


	enum AudioEncoderType
	{
		Opus_Encoder,
		PCM_Encoder,
		No_Encoder
	};


	enum AudioDecoderType
	{
		AAC_Decoder,
		Opus_Decoder,
		PCM_Decoder,
		No_Decoder
	};


	enum AudioGainType
	{
		WebRTC_Gain,
		GomGom_Gain,
		Naive_Gain,
		No_Gain
	};


	enum EchoCancelerType
	{
		WebRTC_ECM,
		Speex_ECM,
		No_ECM
	};


	enum NoiseReducerType
	{
		WebRTC_NoiseReducer,
		No_NoiseReducer
	};


	enum AudioHeaderTypes
	{
		HEADER_COMMON,
		HEADER_CHANNEL,
		HEADER_CALL,
		HEADER_LIVE
	};

	// Max Size of the Informations
	const int iSzOfm_sDeviceInformationNameForLog = 19;

	// Name of the Informations:
	const std::string m_sDeviceInformationNameForLog[iSzOfm_sDeviceInformationNameForLog] = {
		"",											//0
		"Delay",									//1
		"Delay",									//2
		"Delay Fraction",							//3
		"Delay Fraction",							//4
		"Start Up Farend Buffer Size",				//5
		"Start Up Farend Buffer Size",				//6
		"Current Max",								//7	
		"Current Max",								//8
		"Min",										//9
		"Min",										//10
		"Average Time Difference",					//11
		"Average Time Difference",					//12
		"Call in Live",								//13
		"",											//14
		"Call Count",								//15
		"",											//16						
		"Total Data Size",							//17
		"Total Data Size"							//18
	};

	// Byte Size of the Informations
	const int iaDeviceInformationByteSize[iSzOfm_sDeviceInformationNameForLog] = {
		-1,											//0
		2,											//1
		2,											//2
		1,											//3
		1,											//4
		1,											//5
		1,											//6
		1,											//7
		1,											//8
		1,											//9
		1,											//10
		1,											//11
		1,											//12
		1,											//13
		-1,											//14
		1,											//15
		-1,											//16
		4,											//17
		4											//18
	};

	// Index of the Publisher[0] and Callee[1] Informations
	const int iaDeviceInformationDelay[2] = { 1, 2 };
	const int iaDeviceInformationDelayFraction[2] = { 3, 4 };
	const int iaDeviceInformationStartUpFarendBufferSize[2] = { 5, 6 };
	const int iaDeviceInformationCurrentFarendBufferSizeMax[2] = { 7, 8 };
	const int iaDeviceInformationCurrentFarendBufferSizeMin[2] = { 9, 10 };
	const int iaDeviceInformationAverageRecorderTimeDiff[2] = { 11, 12 };
	const int iaDeviceInformationIsCalling = 13;
	const int iaDeviceInformationCountCall = 15;
	const int iaDeviceInformationTotalDataSz[2] = { 17, 18 };

	struct DeviceInformation
	{
		// For Publisher Device Information = 0 index
		// For Callee Device Information = 0 index
		std::unordered_map <int, long long> mDeviceInfo;

		long long llLastTime;
		
		void Reset()
		{
			for (int i = 0; i < 2; i++)
			{
				mDeviceInfo.clear();
				mDeviceInfo[iaDeviceInformationDelay[i]] = SHRT_MAX;
				mDeviceInfo[iaDeviceInformationDelayFraction[i]] = 255;
				mDeviceInfo[iaDeviceInformationCurrentFarendBufferSizeMax[i]] = SHRT_MIN;
				mDeviceInfo[iaDeviceInformationCurrentFarendBufferSizeMin[i]] = SHRT_MAX;
				mDeviceInfo[iaDeviceInformationAverageRecorderTimeDiff[i]] = 0;
				mDeviceInfo[iaDeviceInformationTotalDataSz[i]] = 0;
			}
			llLastTime = -1;
		}

		void ResetAfter(int end = 2)
		{
			if (end > 2) end = 2;
			for (int i = 0; i < end; i++)
			{
				mDeviceInfo[iaDeviceInformationCurrentFarendBufferSizeMax[i]] = SHRT_MIN;
				mDeviceInfo[iaDeviceInformationCurrentFarendBufferSizeMin[i]] = SHRT_MAX;
				mDeviceInfo[iaDeviceInformationAverageRecorderTimeDiff[i]] = 0;
				mDeviceInfo[iaDeviceInformationTotalDataSz[i]] = 0;
			}
		}
	};

} //namespace MediaSDK

#endif  // !AUDIO_TYPES_H
