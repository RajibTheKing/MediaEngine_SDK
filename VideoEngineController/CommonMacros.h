#ifndef COMMON_MACROS_H
#define COMMON_MACROS_H

namespace MediaSDK
{

#define MEDIA_ENGINE_VERSION "9.39.1"
#define MEDIA_ENGINE_BUILD_NUMBER 939012608

/**
* Macros to detect OS
*/
#define MEDIA_OS_UNKNOWN			0x0000
#define MEDIA_OS_WINDOWS_DESKTOP	0x0001
#define MEDIA_OS_WINDOWS_PHONE		0x0002
#define MEDIA_OS_WINDOWS_STORE		0x0004
#define MEDIA_OS_ANDROID			0x0008
#define MEDIA_OS_IPHONE				0x0010
#define MEDIA_OS_LINUX				0x0020
#define MEDIA_OS_UNIX				0x0040
#define MEDIA_OS_MAC				0x0080

#ifdef _WIN32
	//define something for Windows (32-bit and 64-bit, this part is common)
	#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		// This code is for Win32 desktop apps
		#define MEDIA_OS_TYPE	MEDIA_OS_WINDOWS_DESKTOP
		#define OS_TYPE_WINDOWS_DESKTOP
	#else
		// This code is for Windows Store or Windows phone apps
		#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
			// This code is for Windows phone apps only
			#define MEDIA_OS_TYPE	MEDIA_OS_WINDOWS_PHONE
			#define OS_TYPE_WINDOWS_PHONE		
		#endif
	#endif
#ifdef _WIN64
	//define something for Windows (64-bit only)
#else
	//define something for Windows (32-bit only)
#endif

#elif __APPLE__

	#include "TargetConditionals.h"
	#if TARGET_IPHONE_SIMULATOR

		// iOS Simulator
		#define MEDIA_OS_TYPE	MEDIA_OS_IPHONE
		#define OS_TYPE_IPHONE

	#elif TARGET_OS_IPHONE

		// iOS device
		#define MEDIA_OS_TYPE	MEDIA_OS_IPHONE
		#define OS_TYPE_IPHONE

	#elif TARGET_OS_MAC

		// Other kinds of Mac OS
		#define MEDIA_OS_TYPE	MEDIA_OS_MAC
		#define OS_TYPE_MAC
	#else
		#error "Unknown Apple platform"
	#endif

#elif __ANDROID__

	//Android
	#define MEDIA_OS_TYPE	MEDIA_OS_ANDROID
	#define OS_TYPE_ANDROID

#elif __linux__

	// Linux OS
	#define MEDIA_OS_TYPE	MEDIA_OS_LINUX
	#define OS_TYPE_LINUX

#elif __unix__ // all unices not caught above

	// Unix
	#define MEDIA_OS_TYPE	MEDIA_OS_UNIX
	#define OS_TYPE_UNIX

#else

	//This is an alien
	#error "Unknown compiler"
	#define OS_TYPE OS_UNKNOWN
	#define OS_TYPE_UNKNOWN

#endif

//Some useful OS combinations
#define MEDIA_OS_WINDOWS_ALL		(MEDIA_OS_WINDOWS_DESKTOP | MEDIA_OS_WINDOWS_PHONE | MEDIA_OS_WINDOWS_STORE)
#define MEDIA_OS_ANDROID_IPHONE		(MEDIA_OS_ANDROID | MEDIA_OS_IPHONE)
#define MEDIA_OS_IPHONE_ANDROID		MEDIA_OS_ANDROID_IPHONE

#define MEDIA_OS_NON_WINDOWS		(MEDIA_OS_LINUX | MEDIA_OS_ANDROID | MEDIA_OS_IPHONE | MEDIA_OS_MAC)
#define MEDIA_OS_NON_ANDROID		(MEDIA_OS_WINDOWS_ALL | MEDIA_OS_LINUX | MEDIA_OS_IPHONE | MEDIA_OS_MAC)
#define MEDIA_OS_NON_IPHONE			(MEDIA_OS_WINDOWS_ALL | MEDIA_OS_LINUX | MEDIA_OS_MAC)

/**
* Utility macros
*/

/// Macro to check whether the passed MEDIA_OS_* platform is equeal to the build environment 
#define IS_OS(Media_OS)				( (Media_OS) & MEDIA_OS_TYPE )

/*
*  Macro to detect build environment
*/

//Windows
#if _WIN32 || _WIN64
	#if _WIN64
		#define ENV_64BIT
	#else
		#define ENV_32BIT
	#endif
#endif

// Check GCC
#if __GNUC__
	#if __x86_64__ || __ppc64__
		#define ENV_64BIT
	#else
		#define ENV_32BIT
	#endif
#endif

/*
* Macros to decide compiler type
*/
#define COMPILER_CLANG	1
#define COMPILER_GCC	2
#define COMPILER_MSVC	3

/// Mind the order
#if defined(__clang__)
	#define COMPILER_TYPE COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
	#define COMPILER_TYPE COMPILER_GCC
#elif defined(_MSC_VER)
	#define COMPILER_TYPE COMPILER_MSVC
#endif

#if (COMPILER_TYPE == COMPILER_MSVC)
	#define	ForceInline FORCEINLINE ///For VC (Desktop, Windows Phone)
#else
	#define	ForceInline __attribute__((always_inline)) ///For GCC (Linux, Android and iOS)
#endif

//Enable to create log
//#define LOG_ENABLED
//#define BENCHMARK_ENABLED


} // namespace MediaSDK

#endif