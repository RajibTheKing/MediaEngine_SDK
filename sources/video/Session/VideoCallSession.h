
#ifndef IPV_VIDEO_CALL_SESSION_H
#define IPV_VIDEO_CALL_SESSION_H

#include "Size.h"
#include "VideoEncoder.h"
#include "VideoDecoder.h"
#include "VideoEncoderListHandler.h"
#include "CommonTypes.h"
#include "ColorConverter.h"
#include "EncodingBuffer.h"
#include "RenderingBuffer.h"
#include "EncodedFrameDepacketizer.h"
#include "Tools.h"
#include "BitRateController.h"
#include "VideoEncodingThread.h"
#include "VideoEncodingThreadOfCall.h"
#include "VideoEncodingThreadOfLive.h"
#include "RenderingThread.h"
#include "RenderingThreadOfCall.h"
#include "RenderingThreadOfLive.h"
#include "RenderingThreadOfChannel.h"
#include "VideoDecodingThread.h"
#include "VideoDecodingThreadOfCall.h"
#include "VideoDecodingThreadOfLive.h"
#include "VideoDecodingThreadOfChannel.h"
#include "DepacketizationThread.h"
#include "SendingThread.h"
#include "SendingThreadOfCall.h"
#include "SendingThreadOfLive.h"
#include "VersionController.h"
#include "DeviceCapabilityCheckBuffer.h"
#include "FPSController.h"
#include "FrameRateController.h"
#include "LiveReceiver.h"
#include "LiveVideoDecodingQueue.h"
#include "IDRFrameIntervalController.h"

namespace MediaSDK
{


using namespace std;

class CCommonElementsBucket;
class CVideoEncoder;
class CController;

class CVideoCallSession
{

public:

	CVideoCallSession(CController *pController, long long fname, CCommonElementsBucket* sharedObject, int nFPS, int *nrDeviceSupportedCallFPS, bool bIsCheckCall, CDeviceCapabilityCheckBuffer *deviceCheckCapabilityBuffer, int nOwnSupportedResolutionFPSLevel, int nServiceType, int nChannelType, int nEntityType, bool bAudioOnlyLive, bool bSelfViewOnly);
	~CVideoCallSession();

	long long GetFriendID();
	void InitializeVideoSession(long long lFriendID, int iVideoHeight, int iVideoWidth,int nServiceType, int iNetworkType, bool downscaled, int deviceCapability);
	CVideoEncoder* GetVideoEncoder();
	int PushIntoBufferForEncoding(unsigned char *in_data, unsigned int in_size, int device_orientation);
	CVideoDecoder* GetVideoDecoder();
	CColorConverter* GetColorConverter();

	bool PushPacketForMerging(unsigned char *in_data, unsigned int in_size, bool bSelfData, int numberOfFrames = 0, int *frameSizes = NULL, int numberOfMissingFrames = 0, int *missingFrames = NULL);
	bool PushPacketForMergingVector(int offset, unsigned char *in_data, unsigned int in_size, bool bSelfData, int numberOfFrames, int *frameSizes, std::vector< std::pair<int, int> > vMissingFrames);
	CEncodedFrameDepacketizer * GetEncodedFrameDepacketizer();

	void PushFrameForDecoding(unsigned char *in_data, unsigned int frameSize, int nFramNumber, unsigned int timeStampDiff);

	void CreateAndSendMiniPacket(int resendFrameNumber, int resendPacketNumber);
    void CreateAndSend_IDR_Frame_Info_Packet(long long llMissedFrameNumber);
    
	CFPSController* GetFPSController();

	void StartCallInLive(int nCallInLiveType, int nCalleeID);
	void EndCallInLive(int nCalleeID);

	CSendingThreadOfCall *m_pSendingThreadOfCall;
	CSendingThreadOfLive *m_pSendingThreadOfLive;

	CVideoEncodingThreadOfCall *m_pVideoEncodingThreadOfCall;
	CVideoEncodingThreadOfLive *m_pVideoEncodingThreadOfLive;

	CRenderingThreadOfCall *m_pRenderingThreadOfCall;
	CRenderingThreadOfLive *m_pRenderingThreadOfLive;
	CRenderingThreadOfChannel *m_pRenderingThreadOfChannel;

	CVideoDecodingThreadOfCall *m_pVideoDecodingThreadOfCall;
	CVideoDecodingThreadOfLive *m_pVideoDecodingThreadOfLive;
	CVideoDecodingThreadOfChannel *m_pVideoDecodingThreadOfChannel;

	CVideoDepacketizationThread *m_pVideoDepacketizationThread;

	long long GetFirstVideoPacketTime();
	void SetFirstVideoPacketTime(long long llTimeStamp);

	void SetFirstFrameEncodingTime(long long time);
	long long GetFirstFrameEncodingTime();
	void SetShiftedTime(long long llTime);
	long long GetShiftedTime();	

	void SetOwnFPS(int nOwnFPS);
	void SetOpponentFPS(int nOpponentFPS);
    bool GetResolationCheck();
    void SetCalculationStartMechanism(bool s7);
    long long GetCalculationStartTime();
    bool GetCalculationStatus();
    void DecideHighResolatedVideo(bool bValue);
    bool GetHighResolutionSupportStatus();
    
    void SetOpponentHighResolutionSupportStatus(bool bValue);
    bool GetOpponentHighResolutionSupportStatus();
    
    void ReInitializeVideoLibrary(int iHeight, int iWidth);
    bool GetReinitializationStatus();
    void OperationForResolutionControl(unsigned char* in_data, int in_size);
    bool GetResolutionNegotiationStatus();
    CVersionController* GetVersionController();

	void FirstFrameCapturingTimeStamp();

	void StopDeviceAbilityChecking();
    
	int GetOwnVideoCallQualityLevel();
	void SetOwnVideoCallQualityLevel(int nVideoCallQualityLevel);

	int GetOpponentVideoCallQualityLevel();
	void SetOpponentVideoCallQualityLevel(int nVideoCallQualityLevel);
	
	int GetCurrentVideoCallQualityLevel();
	void SetCurrentVideoCallQualityLevel(int nVideoCallQualityLevel);
    int GetServiceType();
	int GetEntityType();

	BitRateController* GetBitRateController();
    bool isLiveVideoStreamRunning();

	void SetCallInLiveType(int nCallInLiveType);

	int SetEncoderHeightWidth(const long long& lFriendID, int height, int width, int nDataType, bool bDownscaled, int deviceCapability);
	int SetDeviceHeightWidth(const long long& lFriendID, int height, int width);

    void SetBeautification(bool bIsEnable);

	void SetVideoQualityForLive(int quality);

	int SetVideoEffect(int nEffectStatus);
	int TestVideoEffect(int *param, int size);

	void SetOwnDeviceType(int deviceType);
	int GetOwnDeviceType();

	void SetOponentDeviceType(int deviceType);
	int GetOponentDeviceType();
    
    void SetOpponentVideoHeightWidth(int iHight, int iWidth);
    int GetOpponentVideoHeight();
    int GetOpponentVideoWidth();

	bool GetAudioOnlyLiveStatus();
	int GetCallInLiveType();

	void InterruptOccured();
	void InterruptOver();
    
    bool isDynamicIDR_Mechanism_Enable();
    
    CAverageCalculator* getFpsCalculator();
    
    int getLiveVideoQualityLevel();
    
    int getVideoCallHeight()
    {
        return m_nVideoCallHeight;
    }
    
    int getVideoCallWidth()
    {
        return m_nVideoCallWidth;
    }

	int getGivenFrameHeight()
	{
		return m_nGivenFrameHeight;
	}

	int getGivenFrameWidth()
	{
		return m_nGivenFrameWidth;
	}

	int GetChannelType();
    
	bool m_bVideoCallStarted;
    CController *m_pController;
	int m_nCallFPS;
    bool m_bLiveVideoStreamRunning;



	CVideoEncodingThread *m_pVideoEncodingThread;
    int m_nVideoCallHeight;
    int m_nVideoCallWidth;
    

private:

	int m_nPublisherInsetNumber;
	bool m_bDownscaled;
	CSendingThread *m_pSendingThread;

	bool m_bFrameReduce;
	
	CVideoRenderingThread *m_pVideoRenderingThread;
	CVideoDecodingThread *m_pVideoDecodingThread;

	int m_nQualityCounter;

	//CVideoDecodingThreadOfLive *m_pVideoDecodingThreadForSecondInset;
	//CVideoDecodingThreadOfLive *m_pVideoDecodingThreadForThirdInset;

	//CVideoDecoder *m_pVideoDecoderForSecondInset;
	//CVideoDecoder *m_pVideoDecoderForThirdInset;

	CFPSController *m_pFPSController;
	CFrameRateController *m_pFrameRateController;
	long long m_LastTimeStampClientFPS;
	double m_ClientFPSDiffSum;
	int m_ClientFrameCounter;
	double m_ClientFPS;
	int m_EncodingFrameCounter;
	bool m_bSkipFirstByteCalculation;

	int m_nDUCounter;

	int m_bLiveVideoQuality;

	bool m_bVideoEffectEnabled;

	int m_nOwnVideoCallQualityLevel;
	int m_nOpponentVideoCallQualityLevel;
	int m_nCurrentVideoCallQualityLevel;

	bool m_bAudioOnlyLive;
	bool m_bVideoOnlyLive;
	bool m_bSelfViewOnly;

	int m_nCallInLiveType;
	int m_nFrameCount;

	int m_nReduceCheckNumber;

	bool m_bDoubleUpdate;

	int m_iRole;

	long long m_llShiftedTime;
	long long m_llTimeStampOfFirstPacketRcvd;
	long long m_nFirstFrameEncodingTimeDiff;
	int m_ByteRcvInBandSlot;
	long long m_llFirstFrameCapturingTimeStamp;

	unsigned int m_miniPacketBandCounter;

	int m_nOwnDeviceType;
	int m_nOponentDeviceType;

	CVideoHeader m_cVH;

	int m_nEntityType;

	int m_nCapturedFrameCounter;

	int m_nDeviceHeight;
	int m_nDeviceWidth;

	long long m_llSlotResetLeftRange;
	long long m_llSlotResetRightRange;

	bool m_bIsCheckCall;

	int *pnDeviceSupportedFPS;
    
    long long mt_llCapturePrevTime;
    bool m_bResolationCheck;
    bool m_bShouldStartCalculation;
    long long m_bCaclculationStartTime;
    bool m_bHighResolutionSupportedForOwn;
    bool m_bHighResolutionSupportedForOpponent;
    bool m_bReinitialized;
    bool m_bResolutionNegotiationDone;
    
    int m_nServiceType;
	int m_nChannelType;
    
	Tools m_Tools;
	long long m_lfriendID;
	CVideoEncoderListHandler sessionMediaList;

	CVideoHeader m_PacketHeader;

	CEncodedFrameDepacketizer *m_pEncodedFrameDepacketizer;
	CEncodedFramePacketizer *m_pEncodedFramePacketizer;
	CCommonElementsBucket* m_pCommonElementsBucket;
	CVideoEncoder *m_pVideoEncoder;
	CVideoDecoder *m_pVideoDecoder;

	BitRateController *m_BitRateController;
    IDRFrameIntervalController *m_pIdrFrameIntervalController;
    

	CEncodingBuffer *m_EncodingBuffer;
	CVideoPacketQueue *m_pVideoPacketQueue;
	CVideoPacketQueue *m_pRetransVideoPacketQueue;
	CVideoPacketQueue *m_pMiniPacketQueue;
	CRenderingBuffer *m_RenderingBuffer;
	CSendingBuffer *m_SendingBuffer;

	CColorConverter *m_pColorConverter;

	unsigned char m_CroppedFrame[MAX_VIDEO_DECODER_FRAME_SIZE];

	unsigned char m_miniPacket[VIDEO_HEADER_LENGTH + 1];

	unsigned char m_ucaReceivedLargeFrame[MAX_VIDEO_FRAME_INPUT_SIZE];
    
    CVersionController *m_pVersionController;
    CDeviceCapabilityCheckBuffer *m_pDeviceCheckCapabilityBuffer = NULL;
    
    int m_nDeviceCheckFrameCounter;
    long long m_llClientFrameFPSTimeStamp;
    CAverageCalculator *m_VideoFpsCalculator;
    
    LiveReceiver *m_pLiveReceiverVideo;
    LiveVideoDecodingQueue *m_pLiveVideoDecodingQueue;
    
    int m_nOpponentVideoHeight;
    int m_nOpponentVideoWidth;

    bool m_bDynamic_IDR_Sending_Mechanism;

	int m_nSmalledFrameHeight;
	int m_nSmalledFrameWidth;
	int m_nGivenFrameHeight;
	int m_nGivenFrameWidth;
    


protected:

	SharedPointer<CLockHandler> m_pVideoCallSessionMutex;
};

} //namespace MediaSDK

#endif
