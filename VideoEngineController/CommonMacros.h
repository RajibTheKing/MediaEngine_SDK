#ifndef COMMON_MACROS_H
#define COMMON_MACROS_H

namespace MediaSDK
{

#define MEDIA_ENGINE_VERSION "9.39.1"
#define MEDIA_ENGINE_BUILD_NUMBER 939012608

	/********Enable to create log************/

#define LOG_ENABLED



//#define BENCHMARK_ENABLED

//#define	LIVE_CHUNK_DUMPLINGS


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

/** 
  Macros to detect processor architecture
*/

#define MEDIA_ARCH__ARM			0x0001
#define MEDIA_ARCH__ARM5		0x0002
#define MEDIA_ARCH__ARM6		0x0004
#define MEDIA_ARCH__ARM7		0x0008
#define MEDIA_ARCH__ARM7a		0x0010
#define MEDIA_ARCH__ARM7r		0x0020
#define MEDIA_ARCH__ARM7m		0x0040
#define MEDIA_ARCH__ARM7s		0x0080
#define MEDIA_ARCH__ARM64		0x0100
#define MEDIA_ARCH__x86			0x0200
#define MEDIA_ARCH__x86_64		0x0400	
#define MEDIA_ARCH__MIPS64		0x0800
#define MEDIA_ARCH__MIPS		0x1000
#define MEDIA_ARCH__UNKNOWN		0x8000

#if defined(COMPILER_MSVC)
	#if defined(_M_ARM)

		#if (_M_ARM == 5)

			#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM5
			#define MEDIA_ARCH_ARMv5
			#define MEDIA_ABI "armeabi-v5"

		#elif (_M_ARM == 6)

			#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM6
			#define MEDIA_ARCH_ARMv6
			#define MEDIA_ABI "armeabi-v6"

		#elif (_M_ARM == 7)

			#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM7
			#define MEDIA_ARCH_ARMv7
			#define MEDIA_ABI "armeabi-v7"

		#else

			#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM
			#define MEDIA_ARCH_ARM
			#define MEDIA_ABI "armeabi"

		#endif

	#elif defined(_M_ARM64)

		#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM64
		#define MEDIA_ARCH_ARM64
		#define MEDIA_ABI "arm64"

	#elif defined(_M_IX86)

		#define MEDIA_ARCH_TYPE	MEDIA_ARCH__x86
		#define MEDIA_ARCH_x86
		#define MEDIA_ABI "x86"

	#elif defined(_M_X64)

		#define MEDIA_ARCH_TYPE	MEDIA_ARCH__x86_64
		#define MEDIA_ARCH_x86_64
		#define MEDIA_ABI "x86_64"

	#else

		#define MEDIA_ARCH_TYPE	MEDIA_ARCH__UNKNOWN
		#define MEDIA_ABI "unknown"

	#endif
#else //GCC, Clang, others
	#if defined(__arm__)

		#if defined(__ARM_ARCH_5__)

			#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM5
			#define MEDIA_ARCH_ARMv5
			#define MEDIA_ABI "armeabi-v5"

		#elif defined(__ARM_ARCH_6__)

			#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM6
			#define MEDIA_ARCH_ARMv6
			#define MEDIA_ABI "armeabi-v6"

		#elif defined(__ARM_ARCH_7__)

			#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM7
			#define MEDIA_ARCH_ARMv7
			#define MEDIA_ABI "armeabi-v7"

		#elif defined(__ARM_ARCH_7A__)

			#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM7a
			#define MEDIA_ARCH_ARMv7a

			#if defined(__ARM_NEON__)

				#define MEDIA_ARCH_NEON
				#if defined(__ARM_PCS_VFP)
					#define MEDIA_ABI "armeabi-v7a/NEON (hard-float)"
				#else
					#define MEDIA_ABI "armeabi-v7a/NEON"
				#endif

			#else

				#if defined(__ARM_PCS_VFP)
					#define MEDIA_ABI "armeabi-v7a (hard-float)"
				#else
					#define MEDIA_ABI "armeabi-v7a"
				#endif

			#endif

		#elif defined(__ARM_ARCH_7R__)

			#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM7r
			#define MEDIA_ARCH_ARMv7r
			#define MEDIA_ABI "armeabi-v7r"

		#elif defined(__ARM_ARCH_7M__)

			#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM7m
			#define MEDIA_ARCH_ARMv7m
			#define MEDIA_ABI "armeabi-v7m"

		#elif defined(__ARM_ARCH_7S__)
			
			#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM7s
			#define MEDIA_ARCH_ARMv7s
			#define MEDIA_ABI "armeabi-v7s"

		#else

			#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM
			#define MEDIA_ARCH_ARM
			#define MEDIA_ABI "armeabi"

		#endif

	#elif defined(__i386__)

		#define MEDIA_ARCH_TYPE	MEDIA_ARCH__x86
		#define MEDIA_ARCH_x86
		#define MEDIA_ABI "x86"
	
	#elif defined(__x86_64__)

		#define MEDIA_ARCH_TYPE	MEDIA_ARCH__x86_64
		#define MEDIA_ARCH_x86_64
		#define MEDIA_ABI "x86_64"

	#elif defined(__mips64)  /* mips64el-* toolchain defines __mips__ too */

		#define MEDIA_ARCH_TYPE	MEDIA_ARCH__MIPS64
		#define MEDIA_ARCH_MIPS64
		#define MEDIA_ABI "mips64"

	#elif defined(__mips__)

		#define MEDIA_ARCH_TYPE	MEDIA_ARCH__MIPS
		#define MEDIA_ARCH_MIPS
		#define MEDIA_ABI "mips"

	#elif defined(__aarch64__)

		#define MEDIA_ARCH_TYPE	MEDIA_ARCH__ARM64
		#define MEDIA_ARCH_ARM64
		#define MEDIA_ABI "arm64-v8a"

	#else

		#define MEDIA_ARCH_TYPE	MEDIA_ARCH__UNKNOWN
		#define MEDIA_ARCH_UNKNOWN
		#define MEDIA_ABI "unknown"

	#endif

#endif

/**
	Useful macro to check platform architecture
*/
#define IS_ARCH(Media_Arch) ( (Media_Arch) & MEDIA_ARCH_TYPE )

#define MIN_CHUNK_DURATION_SAFE 50

} // namespace MediaSDK

#endif