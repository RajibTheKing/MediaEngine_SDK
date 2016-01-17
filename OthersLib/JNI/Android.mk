LOCAL_PATH := $(call my-dir)

ARCHITECTURE := armeabi-v7a
PRECOMPILED_LIBRARIES := ../../../RingIDSDK/jni/precompiled

$(warning $(ARCHITECTURE))
# Prebuilt libssl
include $(CLEAR_VARS)
LOCAL_MODULE := openh264lib
LOCAL_SRC_FILES := $(PRECOMPILED_LIBRARIES)/$(ARCHITECTURE)/libopenh264.a
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
			../../../videoengine/VideoEngineController/SendingBuffer.cpp \
			../../../videoengine/VideoEngineController/RenderingBuffer.cpp \
			../../../videoengine/VideoEngineController/VideoPacketQueue.cpp \
			../../../videoengine/VideoEngineController/VideoEncoderListHandler.cpp \
            ../../../videoengine/VideoEngineController/VideoPacketBuffer.cpp \
            ../../../videoengine/VideoEngineController/EncodedFrameDepacketizer.cpp \
            ../../../videoengine/VideoEngineController/AudioEncoderBuffer.cpp \
            ../../../videoengine/VideoEngineController/AudioDecoderBuffer.cpp \
            ../../../videoengine/VideoEngineController/ResendingBuffer.cpp \
            ../../../videoengine/VideoEngineController/PairMap.cpp \
            ../../../videoengine/VideoEngineController/FPSController.cpp \
            ../../../videoengine/VideoEngineController/Globals.cpp \
            ../../../videoengine/VideoEngineController/DepacketizationBufferIndex.cpp \

include $(BUILD_STATIC_LIBRARY)


#  ringid
include $(CLEAR_VARS)
G729    := g729
LOCAL_MODULE    := ring_codec
LOCAL_SRC_FILES :=  \
	../../../videoengine/include/g729/g729a_decoder.c \
	../../../videoengine/include/g729/g729a_encoder.c \
	../../../videoengine/include/g729/basic_op.c \
	../../../videoengine/include/g729/cod_ld8a.c \
	../../../videoengine/include/g729/bits.c \
	../../../videoengine/include/g729/oper_32b.c \
	../../../videoengine/include/g729/tab_ld8a.c \
	../../../videoengine/include/g729/p_parity.c \
	../../../videoengine/include/g729/dec_ld8a.c \
	../../../videoengine/include/g729/postfilt.c \
	../../../videoengine/include/g729/post_pro.c \
	../../../videoengine/include/g729/pre_proc.c \
	../../../videoengine/include/g729/lpc.c \
	../../../videoengine/include/g729/qua_lsp.c \
	../../../videoengine/include/g729/lpcfunc.c \
	../../../videoengine/include/g729/filter.c \
	../../../videoengine/include/g729/pitch_a.c \
	../../../videoengine/include/g729/dec_lag3.c \
	../../../videoengine/include/g729/taming.c \
	../../../videoengine/include/g729/acelp_ca.c \
	../../../videoengine/include/g729/cor_func.c \
	../../../videoengine/include/g729/qua_gain.c \
	../../../videoengine/include/g729/de_acelp.c \
	../../../videoengine/include/g729/dec_gain.c \
	../../../videoengine/include/g729/dspfunc.c \
	../../../videoengine/include/g729/gainpred.c \
	../../../videoengine/include/g729/lspdec.c \
	../../../videoengine/include/g729/lspgetq.c \
	../../../videoengine/include/g729/round.c \
	../../../videoengine/include/g729/pred_lt3.c \
	../../../videoengine/include/g729/util.c \
	../../../videoengine/include/g729/G729CodecNative.cpp \
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES += ../../../videoengine/include/g729 \
LOCAL_CFLAGS = -O3
include $(BUILD_STATIC_LIBRARY)

#  opennew
include $(CLEAR_VARS)
G729    := g729
LOCAL_MODULE := IPVisionConnectivity
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

LOCAL_SRC_FILES := \
            ../../../RingIDSDK/CInterfaceOfRingSDK.cpp \
            ../../../RingIDSDK/RingIDSDK.cpp \
			../../../RingIDSDK/JNIInterfaceOfRingSDK.cpp \



LOCAL_C_INCLUDES := \
            ../../../videoengine/VideoEngineUtilities \
			../../../videoengine/VideoEngineController \
			../../../videoengine/OthersLib/boost \
			../../../videoengine/OthersLib/WinOpenH264 \
			../../../RingIDSDK \
			../../../videoengine/include/g729 \
			../../../videoengine/include \


LOCAL_CFLAGS := -DANDROID_NDK
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := videoEngineController openh264lib  ring_codec IPVConnectivityDll IPVConnectivityManager IPVSocket IPVStunMessage

include $(BUILD_SHARED_LIBRARY)