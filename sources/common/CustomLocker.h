#ifndef CUSTOM_LOCKER_H
#define CUSTOM_LOCKER_H

#include <mutex>

namespace MediaSDK
{
	typedef std::mutex CLockHandler;

	class BaseMediaLocker {
	public:
		BaseMediaLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~BaseMediaLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class LiveAudioParserForCalleeLocker
	{
	public:
		LiveAudioParserForCalleeLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~LiveAudioParserForCalleeLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class LiveAudioParserForPublisherLocker {
	public:
		LiveAudioParserForPublisherLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~LiveAudioParserForPublisherLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class LiveAudioParserForChannelLocker  {
	public:
		LiveAudioParserForChannelLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~LiveAudioParserForChannelLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class ControllerLocker  {
	public:
		ControllerLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~ControllerLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class ControllerLockerStart  {
	public:
		ControllerLockerStart(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~ControllerLockerStart() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class StartAudioCallInLiveLocker {
	public:
		StartAudioCallInLiveLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~StartAudioCallInLiveLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};
	class EndAudioCallInLiveLocker  {
	public:
		EndAudioCallInLiveLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~EndAudioCallInLiveLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class EncodeAudioFrameLocker  {
	public:
		EncodeAudioFrameLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~EncodeAudioFrameLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class DecodeAudioFrameLocker  {
	public:
		DecodeAudioFrameLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~DecodeAudioFrameLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class CancelAudioDataLocker {
	public:
		CancelAudioDataLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~CancelAudioDataLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class StopAudioCallLocker {
	public:
		StopAudioCallLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~StopAudioCallLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};
	class ControllerLockerStop {
	public:
		ControllerLockerStop(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~ControllerLockerStop() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};
	class StopTestAudioCallLocker {
	public:
		StopTestAudioCallLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~StopTestAudioCallLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};
	class NearEndLocker_1  {
	public:
		NearEndLocker_1(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~NearEndLocker_1() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class NearEndLockerStoreDataForChunk {
	public:
		NearEndLockerStoreDataForChunk(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~NearEndLockerStoreDataForChunk() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class NearEndLockerGetAudioDataToSend {
	public:
		NearEndLockerGetAudioDataToSend(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~NearEndLockerGetAudioDataToSend() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class NearEndLocker_3 {
	public:
		NearEndLocker_3(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~NearEndLocker_3() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class EncodingThreadLocker_1 {
	public:
		EncodingThreadLocker_1(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~EncodingThreadLocker_1() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class EncodingThreadLocker_3 {
	public:
		EncodingThreadLocker_3(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~EncodingThreadLocker_3() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class DecodingThreadLocker_1 {
	public:
		DecodingThreadLocker_1(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~DecodingThreadLocker_1() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class DecodingThreadLocker_3 {
	public:
		DecodingThreadLocker_3(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~DecodingThreadLocker_3() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class SendingThreadLocker_1 {
	public:
		SendingThreadLocker_1(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SendingThreadLocker_1() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class SendingThreadLocker_3 {
	public:
		SendingThreadLocker_3(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SendingThreadLocker_3() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class RenderingThreadLocker_1 {
	public:
		RenderingThreadLocker_1(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~RenderingThreadLocker_1() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class RenderingThreadLocker_3 {
	public:
		RenderingThreadLocker_3(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~RenderingThreadLocker_3() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class DepackThreadLocker_1 {
	public:
		DepackThreadLocker_1(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~DepackThreadLocker_1() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class DepackThreadLocker_3 {
	public:
		DepackThreadLocker_3(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~DepackThreadLocker_3() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class LiveAudioDecodingQueueLock {
	public:
		LiveAudioDecodingQueueLock(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~LiveAudioDecodingQueueLock() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class AudioDecoderBufferLock {
	public:
		AudioDecoderBufferLock(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~AudioDecoderBufferLock() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class AudioEncoderBufferLock {
	public:
		AudioEncoderBufferLock(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~AudioEncoderBufferLock() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class AudioShortBufferLock {
	public:
		AudioShortBufferLock(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~AudioShortBufferLock() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class AudioShortBufferPublisherLock {
	public:
		AudioShortBufferPublisherLock(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~AudioShortBufferPublisherLock() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};
	class AudioListHandlerLock {
	public:
		AudioListHandlerLock(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~AudioListHandlerLock() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};



	class CapabilityLocker {
	public:
		CapabilityLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~CapabilityLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class DepacketizerLocker {
	public:
		DepacketizerLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~DepacketizerLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class EncodingBufferLocker  {
	public:
		EncodingBufferLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~EncodingBufferLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class FPSControllerLocker {
	public:
		FPSControllerLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~FPSControllerLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};


	class LiveReceiverLocker {
	public:
		LiveReceiverLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~LiveReceiverLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class LiveDecodingQueueLocker {
	public:
		LiveDecodingQueueLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~LiveDecodingQueueLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class PairMapLocker {
	public:
		PairMapLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~PairMapLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class RenderingBufferLocker {
	public:
		RenderingBufferLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~RenderingBufferLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class SendingBufferLocker {
	public:
		SendingBufferLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SendingBufferLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class SynchronizedMapLocker {
	public:
		SynchronizedMapLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SynchronizedMapLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class SessionLocker {
	public:
		SessionLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SessionLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class SessionListLocker {
	public:
		SessionListLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SessionListLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class EncoderLocker {
	public:
		EncoderLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~EncoderLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class EncoderListLocker {
	public:
		EncoderListLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~EncoderListLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};
	class MuxingLocker {
	public:
		MuxingLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~MuxingLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class PacketQueueLocker {
	public:
		PacketQueueLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~PacketQueueLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class ColorConverterLocker {
	public:
		ColorConverterLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~ColorConverterLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class BeautyLocker {
	public:
		BeautyLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~BeautyLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class StartCallLocker {
	public:
		StartCallLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~StartCallLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class ReceiveLockerLive {
	public:
		ReceiveLockerLive(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~ReceiveLockerLive() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class ReceiveLockerCall {
	public:
		ReceiveLockerCall(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~ReceiveLockerCall() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class SendLocker {
	public:
		SendLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SendLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class SetEncoderLocker {
	public:
		SetEncoderLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SetEncoderLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class SetBeautifyLocker {
	public:
		SetBeautifyLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SetBeautifyLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class SetCallInLiveTypeLocker {
	public:
		SetCallInLiveTypeLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SetCallInLiveTypeLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class StartCheckCapabilityLocker {
	public:
		StartCheckCapabilityLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~StartCheckCapabilityLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class InterruptOccuredLocker {
	public:
		InterruptOccuredLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~InterruptOccuredLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class InterruptOverLocker {
	public:
		InterruptOverLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~InterruptOverLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class StopTestCallLocker {
	public:
		StopTestCallLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~StopTestCallLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class StopCallLocker {
	public:
		StopCallLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~StopCallLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class UninitLibLocker {
	public:
		UninitLibLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~UninitLibLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class StartCallInLiveLocker {
	public:
		StartCallInLiveLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~StartCallInLiveLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class EndCallInLiveLocker {
	public:
		EndCallInLiveLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~EndCallInLiveLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class StartAudioCallLocker {
	public:
		StartAudioCallLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~StartAudioCallLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class SetVolumeLocker {
	public:
		SetVolumeLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SetVolumeLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class SetLoudSpeakerLocker  {
	public:
		SetLoudSpeakerLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SetLoudSpeakerLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class SetEchoCancellerLocker {
	public:
		SetEchoCancellerLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SetEchoCancellerLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};
	class StartTestAudioCallLocker {
	public:
		StartTestAudioCallLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~StartTestAudioCallLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class PushAudioForDecodingLocker {
	public:
		PushAudioForDecodingLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~PushAudioForDecodingLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};
	class SendAudioDataLocker {
	public:
		SendAudioDataLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~SendAudioDataLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class CRetransmitVideoPacketQueueLocker {
	public:
		CRetransmitVideoPacketQueueLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~CRetransmitVideoPacketQueueLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class CDecodingBufferLocker {
	public:
		CDecodingBufferLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~CDecodingBufferLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class ControllerEncodeVideoFrameLocker {
	public:
		ControllerEncodeVideoFrameLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~ControllerEncodeVideoFrameLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

	class CircularBufferLocker {
	public:
		CircularBufferLocker(CLockHandler& mediaMutex) : m_mutex(mediaMutex)	{ m_mutex.lock(); }
		~CircularBufferLocker() { m_mutex.unlock(); }
	private:
		CLockHandler& m_mutex;
	};

} //namespace MediaSDK


#endif