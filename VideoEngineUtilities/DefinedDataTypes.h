#ifndef _DEFINED_DATA_TYPES_H_
#define _DEFINED_DATA_TYPES_H_

#include <stdio.h>

#ifdef _WIN32

#include <ws2tcpip.h>
#include <winsock2.h> //can also be winsock.h
// Need to link with Ws2_32.lib
#pragma comment(lib,"Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
typedef void* HANDLE;
#endif


#if  defined(WIN32)
//#include <Windows.h>
#define TARG_PLATFORM_WINDOWS
#elif defined(__unix)
//#include <unistd.h>
#define TARG_PLATFORM_LINUX
#else
// Fail if this is a system you have not compensated for:
//#error "Unknown System Type"
#endif


#ifdef _WIN32

typedef __int32 int32_t;
typedef unsigned int u_int32_t;
typedef unsigned short u_int16_t;
typedef int SocketSendReceiveLength;

#else

#include <errno.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

typedef ssize_t SocketSendReceiveLength;

#endif

#endif