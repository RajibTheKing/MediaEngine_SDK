
#include "IPVConnectivityDLLLinker.h"
#include "IPVConnectivityEngine.h"

#ifdef _WIN32
#if !(defined(WINDOWS_UNIVERSAL) || defined(WINDOWS_PHONE_8))
#include <OleCtl.h>
#endif
#endif

IPVType IPVisionConnectivity(const char* sLogFilePath, int logLevel)
{
	return (IPVType)(new CIPVConnectivityEngine(sLogFilePath, logLevel));
}

/*
BOOL APIENTRY DllMain(
HANDLE hModule,
DWORD dwReason,
void* lpReserved)
{
	return TRUE;
}*/