#ifndef __IPV_CONNECTIVITY_DLL_C_INTERFACE_H_
#define __IPV_CONNECTIVITY_DLL_C_INTERFACE_H_

#ifdef WIN32
typedef __int64 IPVLongType;
#else 
typedef long long IPVLongType;
#endif

#include "IPVConnectivityDLL.h"

#include <string.h>
#include <stdlib.h>
#include <string>

#if (_DESKTOP_C_SHARP_)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif


#ifdef __cplusplus
extern "C" {
#endif

	EXPORT int ipv_Init(const IPVLongType lUserID, const char* sLogFileLocation, int logLevel);

	EXPORT int ipv_InitializeLibrary(const IPVLongType lUserID);

	EXPORT int ipv_SetAuthenticationServer(const char* cAuthServerIP, int iAuthServerPort, const char* cAppSessionId);

	EXPORT SessionStatus ipv_CreateSession(const IPVLongType lFriendID, MediaType mediaType, const char*  cRelayServerIP, int iRelayServerPort);

	EXPORT void ipv_SetRelayServerInformation(const IPVLongType lFriendID, MediaType mediaType, const char* cRelayServerIP, int iRelayServerPort);

	EXPORT void ipv_StartP2PCall(const IPVLongType lFriendID, MediaType mediaType, int bCaller);

	EXPORT bool ipv_IsConnectionTypeHostToHost(IPVLongType lFriendID, MediaType mediaType);
	
	EXPORT int ipv_Send(const IPVLongType lFriendID, MediaType mediaType, unsigned char data[], int iLen);

	EXPORT int ipv_SendTo(const IPVLongType lFriendID, MediaType mediaType, unsigned char data[], int iLen, const char* cDestinationIP, int iDestinationPort);
	
	EXPORT int ipv_Recv(const IPVLongType lFriendID, MediaType mediaType, unsigned char* data, int iLen);

	EXPORT const char* ipv_GetSelectedIPAddress(const IPVLongType lFriendID, MediaType mediaType);

	EXPORT int ipv_GetSelectedPort(const IPVLongType lFriendID, MediaType mediaType);
	
	EXPORT int ipv_CloseSession(const IPVLongType lFriendID, MediaType mediaType);

	EXPORT void ipv_Release();

	EXPORT void ipv_InterfaceChanged();

	EXPORT void ipv_SetLogFileLocation(const char* loc);

	EXPORT void ipv_SetNotifyClientMethodCallback(void(*ptr)(int));

	EXPORT void ipv_SetNotifyClientMethodForFriendCallback(void(*ptr)(int, IPVLongType, int));

	EXPORT void ipv_SetNotifyClientMethodWithReceivedBytesCallback(void(*ptr)(int, IPVLongType, int, int, unsigned char*));


#ifdef __cplusplus
}
#endif

#endif