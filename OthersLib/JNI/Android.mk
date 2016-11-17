LOCAL_PATH := $(call my-dir)

ARCHITECTURE := $(TARGET_ARCH_ABI)
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

# Prebuilt Opus
include $(CLEAR_VARS)
LOCAL_MODULE := Opus
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libopus.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt AECM
# include $(CLEAR_VARS)
# LOCAL_MODULE := AECM
# LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libwebrtc_aecm.a
# include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt NS
# include $(CLEAR_VARS)
# LOCAL_MODULE := NS
# LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libwebrtc_ns.a
# include $(PREBUILT_STATIC_LIBRARY)

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

#  VideoEngineController
include $(CLEAR_VARS)
G729    := g729
LOCAL_MODULE := videoEngineController

LOCAL_C_INCLUDES := \
            ../../../videoengine/VideoEngineController \
			../../../videoengine/VideoEngineUtilities \
			../../../RingIDSDK \
			../../../videoengine/OthersLib/boost \
			../../../videoengine/include/ \
			../../../videoengine/include/aecm \

LOCAL_CFLAGS := -DANDROID_NDK -Wno-deprecated -DPAL_ENABLED -D_LINUX -D_INDENT_DB_PRINT -fsigned-char -fno-inline -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -DUSE_JNI -D_POSIX_PER_PROCESS_TIMER_SOURCE -D_PTHREADS -DUNICODE -lssl -lcrypto

LOCAL_SRC_FILES := \
			../../../videoengine/VideoEngineUtilities/LockHandler.cpp \
            ../../../videoengine/VideoEngineUtilities/ColorConverter.cpp \
            ../../../videoengine/VideoEngineController/AudioCallSession.cpp \
            ../../../videoengine/VideoEngineController/AudioCallSessionListHandler.cpp \
            ../../../videoengine/VideoEngineController/AudioCodec.cpp \
            ../../../videoengine/VideoEngineController/CommonElementsBucket.cpp \
			../../../videoengine/VideoEngineController/Controller.cpp \
			../../../videoengine/VideoEngineController/EncodedFramePacketizer.cpp \
			../../../videoengine/VideoEngineController/DecodingBuffer.cpp \
			../../../videoengine/VideoEngineController/EncodingBuffer.cpp \
			../../../videoengine/VideoEngineController/EventNotifier.cpp \
			../../../videoengine/VideoEngineController/InterfaceOFAudioVideoEngine.cpp \
			../../../videoengine/VideoEngineController/LogPrinter.cpp \
			../../../videoengine/VideoEngineController/Tools.cpp \
			../../../videoengine/VideoEngineController/VideoCallSession.cpp \
            ../../../videoengine/VideoEngineController/VideoCallSessionListHandler.cpp \
            ../../../videoengine/VideoEngineController/VideoDecoder.cpp \
			../../../videoengine/VideoEngineController/VideoEncoder.cpp \
			../../../videoengine/VideoEngineController/VideoEncodingThread.cpp \
			../../../videoengine/VideoEngineController/VideoDecodingThread.cpp \
			../../../videoengine/VideoEngineController/RenderingThread.cpp \
			../../../videoengine/VideoEngineController/SendingThread.cpp \
			../../../videoengine/VideoEngineController/DepacketizationThread.cpp \
			../../../videoengine/VideoEngineController/SendingBuffer.cpp \
			../../../videoengine/VideoEngineController/RenderingBuffer.cpp \
			../../../videoengine/VideoEngineController/VideoPacketQueue.cpp \
			../../../videoengine/VideoEngineController/SynchronizedMap.cpp \
			../../../videoengine/VideoEngineController/VideoEncoderListHandler.cpp \
            ../../../videoengine/VideoEngineController/VideoPacketBuffer.cpp \
            ../../../videoengine/VideoEngineController/EncodedFrameDepacketizer.cpp \
            ../../../videoengine/VideoEngineController/AudioEncoderBuffer.cpp \
            ../../../videoengine/VideoEngineController/AudioDecoderBuffer.cpp \
            ../../../videoengine/VideoEngineController/ResendingBuffer.cpp \
            ../../../videoengine/VideoEngineController/PairMap.cpp \
            ../../../videoengine/VideoEngineController/FPSController.cpp \
            ../../../videoengine/VideoEngineController/Globals.cpp \
            ../../../videoengine/VideoEngineController/PacketHeader.cpp \
			../../../videoengine/VideoEngineController/DepacketizationBufferIndex.cpp \
			../../../videoengine/VideoEngineController/BandwidthController.cpp \
			../../../videoengine/VideoEngineController/BitRateController.cpp \
			../../../videoengine/VideoEngineController/AverageCalculator.cpp \
			../../../videoengine/VideoEngineController/AudioFileEncodeDecodeSession.cpp \
			../../../videoengine/VideoEngineController/VersionController.cpp \
			../../../videoengine/VideoEngineController/DeviceCapabilityCheckBuffer.cpp \
			../../../videoengine/VideoEngineController/DeviceCapabilityCheckThread.cpp \
			../../../videoengine/VideoEngineController/AudioPacketHeader.cpp \
			../../../videoengine/VideoEngineController/AudioFileCodec.cpp \
			../../../videoengine/VideoEngineController/LiveReceiver.cpp \
			../../../videoengine/VideoEngineController/LiveVideoDecodingQueue.cpp \
			../../../videoengine/VideoEngineController/LiveAudioDecodingQueue.cpp \


include $(BUILD_STATIC_LIBRARY)

