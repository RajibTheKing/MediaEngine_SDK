#ifndef COMMON_MACROS_H
#define COMMON_MACROS_H

#include "PlatformMacros.h"

namespace MediaSDK
{

#define MEDIA_ENGINE_VERSION "12.4"
#define MEDIA_ENGINE_BUILD_NUMBER 1204002803

	/********Enable to create log************/

//#define LOG_ENABLED

	/*******ENABLE MEDIA LOGGER *******/
#ifdef LOG_ENABLED
	//#define MEDIA_LOGGER_ENABLED
#endif

//#define BENCHMARK_ENABLED

//#define	LIVE_CHUNK_DUMPLINGS


} // namespace MediaSDK

#endif