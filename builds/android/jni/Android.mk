LOCAL_PATH := $(call my-dir)
ARCH := $(TARGET_ARCH_ABI)

THIRD_PARTY := ../../../third_party
SOURCES := ../../../sources

$(warning $(ARCH))
# Prebuilt libopenh264
include $(CLEAR_VARS)
LOCAL_MODULE := openh264lib
LOCAL_SRC_FILES := $(THIRD_PARTY)/openH264/libs/android/$(ARCH)/libopenh264.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt AAC
include $(CLEAR_VARS)
LOCAL_MODULE := AAC
LOCAL_SRC_FILES := $(THIRD_PARTY)/aac/libs/android/$(ARCH)/libFraunhoferAAC.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt Opus
include $(CLEAR_VARS)
LOCAL_MODULE := Opus
LOCAL_SRC_FILES := $(THIRD_PARTY)/opus/libs/android/$(ARCH)/libopus.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt AECM
include $(CLEAR_VARS)
LOCAL_MODULE := AECM
LOCAL_SRC_FILES := $(THIRD_PARTY)/webrtc/libs/android/$(ARCH)/libwebrtc_aecm.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt NS
include $(CLEAR_VARS)
LOCAL_MODULE := NS
LOCAL_SRC_FILES := $(THIRD_PARTY)/webrtc/libs/android/$(ARCH)/libwebrtc_ns.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt AGC
include $(CLEAR_VARS)
LOCAL_MODULE := AGC
LOCAL_SRC_FILES := $(THIRD_PARTY)/webrtc/libs/android/$(ARCH)/libwebrtc_agc.a
include $(PREBUILT_STATIC_LIBRARY)

# Prebuilt VAD
# include $(CLEAR_VARS)
# LOCAL_MODULE := VAD
# LOCAL_SRC_FILES := $(THIRD_PARTY)/webrtc/libs/android/$(ARCH)/libwebrtc_vad.a
# include $(PREBUILT_STATIC_LIBRARY)

define traverse
  $(wildcard $(1)) $(foreach e, $(wildcard $(1)/*), $(call traverse, $(e)))
endef

# VideoEngine Static Lib
include $(CLEAR_VARS)
LOCAL_MODULE := videoEngineController

LOCAL_CFLAGS := -DANDROID_NDK -Wno-deprecated -DPAL_ENABLED -D_LINUX -D_INDENT_DB_PRINT -fsigned-char -fno-inline -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -DUSE_JNI -D_POSIX_PER_PROCESS_TIMER_SOURCE -D_PTHREADS -DUNICODE -lssl -lcrypto

ALL_FILE := $(call traverse, $(SOURCES))
LOCAL_SRC_FILES := $(filter %.cpp, $(ALL_FILE))
LOCAL_C_INCLUDES := $(filter-out %.cpp %.h, $(ALL_FILE))
LOCAL_C_INCLUDES += \
			$(THIRD_PARTY)/opus/include \
			$(THIRD_PARTY)/webrtc/include \
			$(THIRD_PARTY)/speex/include

include $(BUILD_STATIC_LIBRARY)

# VideoEngine with RingIDSDK
include $(CLEAR_VARS)
LOCAL_MODULE := RingIDSDK
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

RINGID_SDK := ../../../../RingIDSDK

LOCAL_SRC_FILES := \
            $(RINGID_SDK)/CInterfaceOfRingSDK.cpp \
            $(RINGID_SDK)/RingIDSDK.cpp \
			$(RINGID_SDK)/JNIInterfaceOfRingSDK.cpp \

LOCAL_C_INCLUDES := \
            ../../VideoEngineUtilities \
			../../VideoEngineController \
			../../OthersLib/WinOpenH264 \
			../../../RingIDSDK \
			../../include/g729 \
			../../include \

LOCAL_CFLAGS := -DANDROID_NDK
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := videoEngineController openh264lib  ring_codec AAC Opus AGC AECM NS SPEEXAECM IPVConnectivityDll IPVConnectivityManager IPVSocket IPVStunMessage

include $(BUILD_SHARED_LIBRARY)