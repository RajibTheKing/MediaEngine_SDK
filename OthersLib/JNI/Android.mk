LOCAL_PATH := $(call my-dir)

ARCHITECTURE := $(TARGET_ARCH_ABI)
PRECOMPILED_LIBRARIES := ../../../RingIDSDK/jni/precompiled

$(warning $(ARCHITECTURE))


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

LOCAL_CFLAGS := -DANDROID_NDK -Wno-deprecated -DPAL_ENABLED -D_LINUX -D_INDENT_DB_PRINT -fsigned-char -fno-inline -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -DUSE_JNI -D_POSIX_PER_PROCESS_TIMER_SOURCE -D_PTHREADS -DUNICODE -lssl -lcrypto

LOCAL_SRC_FILES := \
			../../../videoengine/VideoEngineUtilities/LockHandler.cpp \
            ../../../videoengine/VideoEngineUtilities/ColorConverter.cpp \
            ../../../videoengine/VideoEngineController/AudioCallSession.cpp \
            ../../../videoengine/VideoEngineController/AudioCallSessionListHandler.cpp \
            ../../../videoengine/VideoEngineController/AudioDecoder.cpp \
            ../../../videoengine/VideoEngineController/AudioEncoder.cpp \
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

include $(BUILD_STATIC_LIBRARY)

