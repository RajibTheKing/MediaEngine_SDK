NDK_TOOLCHAIN_VERSION := 4.8
APP_STL := gnustl_static
APP_OPTIM := release
#APP_ABI = armeabi armeabi-v7a
APP_ABI = armeabi-v7a
APP_CPPFLAGS := -std=c++11
APP_CPPFLAGS += -fexceptions -O2
LOCAL_C_INCLUDES += $(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.8/include
