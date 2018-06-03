#ifndef COMMON_MACROS_H
#define COMMON_MACROS_H

#include "PlatformMacros.h"

namespace MediaSDK
{

#define MEDIA_ENGINE_VERSION "13.1"
#define MEDIA_ENGINE_BUILD_NUMBER 1301003528

	/********Enable to create log************/

//#define LOG_ENABLED

	/*******ENABLE MEDIA LOGGER *******/
#ifdef LOG_ENABLED
	#define MEDIA_LOGGER_ENABLED
#endif

#define MEDIA_LOGGER_LOG_LEVEL LOG_DEBUG


//#define BENCHMARK_ENABLED

#ifdef LOG_ENABLED
	/// Code inside the braces of this macro shall be vanished when log is disabled
	#define LogBlock(logSpecificCodes) logSpecificCodes

	/// Code inside the braces of this macro shall be vanished when log is disabled
	/// Additionally this macro shall create it's own scope, so variables declared inside this macro scope is deleted
	#define LogScope(logSpecificCodes) {logSpecificCodes}
#else
	/// Code inside the braces of this macro shall be vanished when log is disabled
	#define LogBlock(logSpecificCodes)

	/// Code inside the braces of this macro shall be vanished when log is disabled
	/// Additionally this macro shall create it's own scope, so variables declared inside this macro scope is deleted
	#define LogScope(logSpecificCodes)
#endif

//#define	CHUNK_DUMPER

#define MIN_CHUNK_DURATION_SAFE 50

} // namespace MediaSDK

#endif
