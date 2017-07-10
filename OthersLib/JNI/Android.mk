
#START_BUILDING_MEDIAENGINE
LOCAL_PATH := $(call my-dir)

ARCHITECTURE := $(TARGET_ARCH_ABI)
#END_BUILDING_MEDIAENGINE

PRECOMPILED_LIBRARIES := ../../../RingIDSDK/jni/precompiled

$(warning $(ARCHITECTURE))
# Prebuilt libssl
include $(CLEAR_VARS)
LOCAL_MODULE := openh264lib
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libopenh264.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt FileTransfer
include $(CLEAR_VARS)
LOCAL_MODULE := FileTransfer
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libFileTransfer.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt IPVConnectivityDll
include $(CLEAR_VARS)
LOCAL_MODULE := IPVConnectivityDll
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libIPVConnectivityDll.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt IPVConnectivityManager
include $(CLEAR_VARS)
LOCAL_MODULE := IPVConnectivityManager
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libIPVConnectivityManager.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt IPVSocket
include $(CLEAR_VARS)
LOCAL_MODULE := IPVSocket
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libIPVSocket.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt IPVStunMessage
include $(CLEAR_VARS)
LOCAL_MODULE := IPVStunMessage
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libIPVStunMessage.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt AAC
include $(CLEAR_VARS)
LOCAL_MODULE := AAC
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libFraunhoferAAC.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt Opus
include $(CLEAR_VARS)
LOCAL_MODULE := Opus
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libopus.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt AECM
include $(CLEAR_VARS)
LOCAL_MODULE := AECM
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libwebrtc_aecm.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt SPEEXAECM
include $(CLEAR_VARS)
LOCAL_MODULE := SPEEXAECM
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libSpeexAECM.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt NS
include $(CLEAR_VARS)
LOCAL_MODULE := NS
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libwebrtc_ns.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt AGC
include $(CLEAR_VARS)
LOCAL_MODULE := AGC
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libwebrtc_agc.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt VAD
# include $(CLEAR_VARS)
# LOCAL_MODULE := VAD
# LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libwebrtc_vad.a
# include $(PREBUILT_STATIC_LIBRARY)

#START_BUILDING_MEDIAENGINE

#  VideoEngineController
include $(CLEAR_VARS)
G729    := g729
LOCAL_MODULE := videoEngineController

LOCAL_C_INCLUDES := \
            ../../VideoEngineController \
			../../VideoEngineUtilities \
			../../../RingIDSDK \
			../../OthersLib/boost \
			../../include/ \
			../../include/aecm \
			../../include/aecm/webrtc \

LOCAL_CFLAGS := -DANDROID_NDK -Wno-deprecated -DPAL_ENABLED -D_LINUX -D_INDENT_DB_PRINT -fsigned-char -fno-inline -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -DUSE_JNI -D_POSIX_PER_PROCESS_TIMER_SOURCE -D_PTHREADS -DUNICODE -lssl -lcrypto

LOCAL_SRC_FILES := \
			../../VideoEngineController/AudioLinearBuffer.cpp \
			../../VideoEngineController/MuxHeader.cpp \
			../../VideoEngineController/AudioShortBufferForPublisherFarEnd.cpp \
			../../VideoEngineController/AudioPacketizer.cpp \
			../../VideoEngineController/AudioDePacketizer.cpp \
			../../VideoEngineController/VideoSockets.cpp \
			../../VideoEngineController/Filt.cpp \
			../../VideoEngineController/AudioNearEndDataProcessor.cpp \
			../../VideoEngineController/AudioFarEndDataProcessor.cpp \
            ../../VideoEngineUtilities/ColorConverter.cpp \
            ../../VideoEngineUtilities/HashGenerator.cpp \
            ../../VideoEngineController/AudioCallSession.cpp \
            ../../VideoEngineController/AudioCallSessionListHandler.cpp \
            ../../VideoEngineController/CommonElementsBucket.cpp \
			../../VideoEngineController/Controller.cpp \
			../../VideoEngineController/EncodedFramePacketizer.cpp \
			../../VideoEngineController/DecodingBuffer.cpp \
			../../VideoEngineController/EncodingBuffer.cpp \
			../../VideoEngineController/EventNotifier.cpp \
			../../VideoEngineController/InterfaceOFAudioVideoEngine.cpp \
			../../VideoEngineController/IDRFrameIntervalController.cpp \
			../../VideoEngineController/LogPrinter.cpp \
			../../VideoEngineController/Tools.cpp \
			../../VideoEngineController/VideoCallSession.cpp \
            ../../VideoEngineController/VideoCallSessionListHandler.cpp \
            ../../VideoEngineController/VideoDecoder.cpp \
			../../VideoEngineController/VideoEncoder.cpp \
			../../VideoEngineController/VideoEncodingThread.cpp \
			../../VideoEngineController/VideoEncodingThreadOfCall.cpp \
			../../VideoEngineController/VideoEncodingThreadOfLive.cpp \
			../../VideoEngineController/VideoDecodingThread.cpp \
			../../VideoEngineController/VideoDecodingThreadOfCall.cpp \
			../../VideoEngineController/VideoDecodingThreadOfLive.cpp \
			../../VideoEngineController/VideoDecodingThreadOfChannel.cpp \
			../../VideoEngineController/RenderingThread.cpp \
			../../VideoEngineController/RenderingThreadOfCall.cpp \
			../../VideoEngineController/RenderingThreadOfLive.cpp \
			../../VideoEngineController/RenderingThreadOfChannel.cpp \
			../../VideoEngineController/SendingThread.cpp \
			../../VideoEngineController/SendingThreadOfCall.cpp \
			../../VideoEngineController/SendingThreadOfLive.cpp \
			../../VideoEngineController/DepacketizationThread.cpp \
			../../VideoEngineController/SendingBuffer.cpp \
			../../VideoEngineController/RenderingBuffer.cpp \
			../../VideoEngineController/VideoPacketQueue.cpp \
			../../VideoEngineController/SynchronizedMap.cpp \
			../../VideoEngineController/VideoEncoderListHandler.cpp \
            ../../VideoEngineController/VideoPacketBuffer.cpp \
            ../../VideoEngineController/EncodedFrameDepacketizer.cpp \
            ../../VideoEngineController/AudioEncoderBuffer.cpp \
            ../../VideoEngineController/AudioDecoderBuffer.cpp \
            ../../VideoEngineController/ResendingBuffer.cpp \
            ../../VideoEngineController/PairMap.cpp \
            ../../VideoEngineController/FPSController.cpp \
            ../../VideoEngineController/Globals.cpp \
            ../../VideoEngineController/PacketHeader.cpp \
			../../VideoEngineController/DepacketizationBufferIndex.cpp \
			../../VideoEngineController/BandwidthController.cpp \
			../../VideoEngineController/BitRateController.cpp \
			../../VideoEngineController/AverageCalculator.cpp \
			../../VideoEngineController/AudioFileEncodeDecodeSession.cpp \
			../../VideoEngineController/VersionController.cpp \
			../../VideoEngineController/DeviceCapabilityCheckBuffer.cpp \
			../../VideoEngineController/DeviceCapabilityCheckThread.cpp \
			../../VideoEngineController/AudioPacketHeader.cpp \
			../../VideoEngineController/AudioFileCodec.cpp \
			../../VideoEngineController/Voice.cpp \
			../../VideoEngineController/AudioMixer.cpp \
			../../VideoEngineController/LiveAudioParserForCallee.cpp \
			../../VideoEngineController/LiveAudioParserForChannel.cpp \
			../../VideoEngineController/LiveAudioParserForPublisher.cpp \
			../../VideoEngineUtilities/VideoBeautificationer.cpp \
			../../VideoEngineController/LiveReceiver.cpp \
			../../VideoEngineController/LiveVideoDecodingQueue.cpp \
			../../VideoEngineController/LiveAudioDecodingQueue.cpp \
			../../VideoEngineController/VideoMuxingAndEncodeSession.cpp \
			../../VideoEngineController/VideoHeader.cpp \
			../../VideoEngineController/LiveStreamingHeader.cpp \
			../../VideoEngineUtilities/MuxingVideoData.cpp \
			../../VideoEngineUtilities/VideoEffects.cpp \
			../../VideoEngineController/DecoderPCM.cpp \
			../../VideoEngineController/AudioDecoderProvider.cpp \
			../../VideoEngineController/WebRTCGain.cpp \
			../../VideoEngineController/GomGomGain.cpp \
			../../VideoEngineController/NaiveGain.cpp \
			../../VideoEngineController/AudioGainInstanceProvider.cpp \
			../../VideoEngineController/EchoCancellerProvider.cpp \
			../../VideoEngineController/SpeexEchoCanceller.cpp \
			../../VideoEngineController/WebRTCEchoCanceller.cpp \
			../../VideoEngineController/WebRTCNoiseReducer.cpp \
			../../VideoEngineController/NoiseReducerProvider.cpp \
			../../VideoEngineController/AudioSessionOptions.cpp \
			../../VideoEngineController/AudioResources.cpp \
			../../VideoEngineController/AudioNearEndProcessorThread.cpp \
			../../VideoEngineController/AudioNearEndProcessorPublisher.cpp \
			../../VideoEngineController/AudioNearEndProcessorViewer.cpp \
			../../VideoEngineController/AudioNearEndProcessorCall.cpp \
			../../VideoEngineController/AudioFarEndProcessorThread.cpp \
			../../VideoEngineController/AudioFarEndProcessorPublisher.cpp \
			../../VideoEngineController/AudioFarEndProcessorViewer.cpp \
			../../VideoEngineController/AudioFarEndProcessorChannel.cpp \
			../../VideoEngineController/AudioFarEndProcessorCall.cpp \
			../../VideoEngineController/AudioEncoderProvider.cpp \
			../../VideoEngineController/AudioHeaderCommon.cpp \
			../../VideoEngineController/DecoderAAC.cpp \
			../../VideoEngineController/DecoderOpus.cpp \
			../../VideoEngineController/EncoderPCM.cpp \
			../../VideoEngineController/EncoderOpus.cpp \
			../../VideoEngineController/Trace.cpp \


			


include $(BUILD_STATIC_LIBRARY)


#END_BUILDING_MEDIAENGINE

#  ringid
include $(CLEAR_VARS)
G729    := g729
LOCAL_MODULE    := ring_codec
LOCAL_SRC_FILES :=  \
	../../include/g729/g729a_decoder.c \
	../../include/g729/g729a_encoder.c \
	../../include/g729/basic_op.c \
	../../include/g729/cod_ld8a.c \
	../../include/g729/bits.c \
	../../include/g729/oper_32b.c \
	../../include/g729/tab_ld8a.c \
	../../include/g729/p_parity.c \
	../../include/g729/dec_ld8a.c \
	../../include/g729/postfilt.c \
	../../include/g729/post_pro.c \
	../../include/g729/pre_proc.c \
	../../include/g729/lpc.c \
	../../include/g729/qua_lsp.c \
	../../include/g729/lpcfunc.c \
	../../include/g729/filter.c \
	../../include/g729/pitch_a.c \
	../../include/g729/dec_lag3.c \
	../../include/g729/taming.c \
	../../include/g729/acelp_ca.c \
	../../include/g729/cor_func.c \
	../../include/g729/qua_gain.c \
	../../include/g729/de_acelp.c \
	../../include/g729/dec_gain.c \
	../../include/g729/dspfunc.c \
	../../include/g729/gainpred.c \
	../../include/g729/lspdec.c \
	../../include/g729/lspgetq.c \
	../../include/g729/round.c \
	../../include/g729/pred_lt3.c \
	../../include/g729/util.c \
	../../include/g729/G729CodecNative.cpp \
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES += ../../include/g729 \
LOCAL_CFLAGS = -O3
include $(BUILD_STATIC_LIBRARY)

#  opennew
include $(CLEAR_VARS)
G729    := g729
LOCAL_MODULE := RingIDSDK
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

LOCAL_SRC_FILES := \
            ../../../RingIDSDK/CInterfaceOfRingSDK.cpp \
            ../../../RingIDSDK/RingIDSDK.cpp \
			../../../RingIDSDK/JNIInterfaceOfRingSDK.cpp \



LOCAL_C_INCLUDES := \
            ../../VideoEngineUtilities \
			../../VideoEngineController \
			../../OthersLib/boost \
			../../OthersLib/WinOpenH264 \
			../../../RingIDSDK \
			../../include/g729 \
			../../include \


LOCAL_CFLAGS := -DANDROID_NDK
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := videoEngineController openh264lib  ring_codec AAC Opus AGC AECM NS SPEEXAECM IPVConnectivityDll IPVConnectivityManager IPVSocket FileTransfer IPVStunMessage

include $(BUILD_SHARED_LIBRARY)