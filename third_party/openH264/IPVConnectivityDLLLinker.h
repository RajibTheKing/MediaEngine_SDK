#ifndef __IPVConnectivityDLLLinker_H_
#define __IPVConnectivityDLLLinker_H_

#include <stdio.h>
#include <string>

#ifdef WIN32
typedef __int64 IPVLongType;
#else 
typedef long long IPVLongType;
#endif

#if defined (WIN64)
typedef long long IPVType;
typedef unsigned long long u_IPVType;
#elif defined (WIN32)
typedef long IPVType;
typedef unsigned long u_IPVType;
#elif (__LP64__)
typedef long IPVType;
typedef unsigned long u_IPVType;
#elif (__LP32__)
typedef long IPVType;
typedef unsigned long u_IPVType;
#else
typedef long long IPVType;
typedef unsigned long long u_IPVType;

#endif

#ifndef _WIN32
extern "C" {
#endif
	IPVType IPVisionConnectivity(const char* sLogFilePath, int logLevel);
#ifndef _WIN32
}
#endif
#endif