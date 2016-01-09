#ifndef __IPVConnectivity_DLL_H_
#define __IPVConnectivity_DLL_H_

#include <stdio.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "IPVConnectivityInterface.h"
#include "IPVConnectivityDLLLinker.h"


#ifdef _WIN32
#define IPVisionLIB "IPVConnectivity.dll"
#elif __ANDROID__
#define IPVisionLIB "libIPVConnectivityDll.so"
#elif __linux__
#define IPVisionLIB "./libipvConnectivity.so"
#elif __APPLE__
#define IPVisionLIB "libIPVConnectivityDll.dylib"
#endif

#define IPV_DLL_NAME "IPVisionConnectivity"

//typedef IPVType(WINAPI *ipv_pointer)(void);
//typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef IPVType(*ipv_pointer)(const char*, int);



class CIPVConnectivityDLL
{
public:
	CIPVConnectivityDLL() :
		m_pIPVConnectivityInterface(NULL),
		handler(NULL)
	{}

	~CIPVConnectivityDLL() {}

	bool Init(const IPVLongType& lUserID, const std::string& sLogFilePath, int logLevel)
	{
		std::string path = "";

		bool bRet = Load(path, sLogFilePath, logLevel);

		if (bRet)
		{
			bRet &= SetUserName(lUserID);
		}
		return bRet;
	}

	bool InitializeLibrary(const IPVLongType& lUserID)
	{
		std::string path = "";

		bool bRet = Load(path, "IPvisionConnectivityEngine.log", 5);

		if (bRet)
		{
			bRet &= SetUserName(lUserID);
		}
		return bRet;
	}

#ifdef _WIN32
	bool InitDLL(HINSTANCE hInst, const IPVLongType& lUserID, const std::string& sLogFilePath, int& logLevel)
	{
		std::string path = GetPathName(GetModuleDir(hInst), IPVisionLIB);

		bool bRet = Load(path, sLogFilePath, logLevel);

		if (bRet)
		{
			bRet &= SetUserName(lUserID);
		}
		return bRet;
	}

	bool InitializeDLL(HINSTANCE hInst, const IPVLongType& lUserID)
	{
		std::string path = GetPathName(GetModuleDir(hInst), IPVisionLIB);

		bool bRet = Load(path, "IPvisionConnectivityEngine.log", 5);

		if (bRet)
		{
			bRet &= SetUserName(lUserID);
		}
		return bRet;
	}

#endif

	bool SetAuthenticationServer(const std::string& sAuthServerIP, int iAuthServerPort, const std::string& sAppSessionId)
	{
		return (m_pIPVConnectivityInterface) ? m_pIPVConnectivityInterface->SetAuthenticationServer(sAuthServerIP, iAuthServerPort, sAppSessionId) : false;
	}

	SessionStatus CreateSession(const IPVLongType& lFriendID, MediaType mediaType, const std::string& sRelayServerIP, int iRelayServerPort)
	{
		return (m_pIPVConnectivityInterface) ? m_pIPVConnectivityInterface->CreateSession(lFriendID, mediaType, sRelayServerIP, iRelayServerPort) : NONE;
	}

	void SetRelayServerInformation(const IPVLongType& lFriendID, MediaType mediaType, const std::string& sRelayServerIP, int iRelayServerPort)
	{
		if (m_pIPVConnectivityInterface)
		{
			m_pIPVConnectivityInterface->SetRelayServerInformation(lFriendID, mediaType, sRelayServerIP, iRelayServerPort);
		}
	}

	void StartP2PCall(const IPVLongType& lFriendID, MediaType medaiType, bool bCaller)
	{
		if (m_pIPVConnectivityInterface)
		{
			m_pIPVConnectivityInterface->StartP2PCall(lFriendID, medaiType, bCaller);
		}
	}

	bool IsConnectionTypeHostToHost(IPVLongType lFriendID, MediaType mediaType)
	{
		if (m_pIPVConnectivityInterface)
		{
			return m_pIPVConnectivityInterface->IsConnectionTypeHostToHost(lFriendID, mediaType);
		}
		return false;
	}

	int Send(const IPVLongType& lFriendID, const MediaType mediaType, unsigned char data[], int iLen)
	{
		return (m_pIPVConnectivityInterface) ? m_pIPVConnectivityInterface->Send(lFriendID, mediaType, data, iLen) : -1;
	}

	int SendTo(const IPVLongType& lFriendID, const MediaType mediaType, unsigned char data[], int iLen, const std::string& sDestinationIP, int iDestinationPort)
	{
		return (m_pIPVConnectivityInterface) ? m_pIPVConnectivityInterface->SendTo(lFriendID, mediaType, data, iLen, sDestinationIP, iDestinationPort) : -1;
	}

	int Recv(const IPVLongType& lFriendID, MediaType mediaType, unsigned char* data, int iLen)
	{
		return (m_pIPVConnectivityInterface) ? m_pIPVConnectivityInterface->Recv(lFriendID, mediaType, data, iLen) : -1;
	}

	std::string GetSelectedIPAddress(const IPVLongType& lFriendID, MediaType mediaType)
	{
		return (m_pIPVConnectivityInterface) ? m_pIPVConnectivityInterface->GetSelectedIPAddress(lFriendID, mediaType) : "";
	}

	int GetSelectedPort(const IPVLongType& lFriendID, MediaType mediaType)
	{
		return (m_pIPVConnectivityInterface) ? m_pIPVConnectivityInterface->GetSelectedPort(lFriendID, mediaType) : -1;
	}

	bool CloseSession(const IPVLongType& lFriendID, MediaType mediaType)
	{
		return (m_pIPVConnectivityInterface) ? m_pIPVConnectivityInterface->CloseSession(lFriendID, mediaType) : false;
	}

	void Release()
	{
		if (m_pIPVConnectivityInterface)
		{
			m_pIPVConnectivityInterface->Release();
		}
	}

	void InterfaceChanged()
	{
		if (m_pIPVConnectivityInterface)
		{
			m_pIPVConnectivityInterface->InterfaceChanged();
		}
	}

	void SetLogFileLocation(const std::string& loc)
	{
		if (m_pIPVConnectivityInterface)
		{
			m_pIPVConnectivityInterface->SetLogFileLocation(loc);
		}
	}

    void SetNotifyClientMethodCallback(void (*ptr)(int))
	{
		if (m_pIPVConnectivityInterface)
		{
			m_pIPVConnectivityInterface->SetNotifyClientMethodCallback(ptr);
		}
	}

    void SetNotifyClientMethodForFriendCallback(void (*ptr)(int, IPVLongType, int))
	{
		if (m_pIPVConnectivityInterface)
		{
			m_pIPVConnectivityInterface->SetNotifyClientMethodForFriendCallback(ptr);
		}
	}

    void SetNotifyClientMethodWithReceivedBytesCallback(void (*ptr)(int, IPVLongType, int, int, unsigned char*))
	{
		if (m_pIPVConnectivityInterface)
		{
			m_pIPVConnectivityInterface->SetNotifyClientMethodWithReceivedBytesCallback(ptr);
		}
	}

private:
	CIPVConnectivityInterface* m_pIPVConnectivityInterface;
#ifdef _WIN32
	HINSTANCE handler;
#else
	void *handler;
#endif

#ifdef _WIN32
	std::string GetModuleDir(HMODULE hInst)
	{
		char szFileName[MAX_PATH];
		bool bRet = false;
#if !(defined(WINDOWS_UNIVERSAL) || defined(WINDOWS_PHONE_8))
		bRet = (GetModuleFileNameA(
			hInst,
			szFileName,
			sizeof(szFileName) / sizeof(char)) != 0);
#endif

		if (!bRet)
		{
			return "";
		}

		std::string sPathName = szFileName;
		std::string::size_type idx = sPathName.find_last_of('\\');
		if (idx == std::string::npos)
		{
			return "";
		}

		return sPathName.substr(0, idx);
	}

	std::string GetPathName(const std::string& sDirName, const std::string& sFileName)
	{
		if (sFileName.find_first_of('\\') != std::string::npos)
		{
			//filename contains backslash;
			return "";
		}

		if (sDirName.empty())
		{
			return sFileName;
		}

		if (sFileName.empty())
		{
			return sDirName;
		}

		std::string sRet = sDirName;
		if (sDirName[sDirName.length() - 1] != '\\')
		{
			sRet += '\\';
		}
		sRet += sFileName;

		return sRet;
	}
#endif
	bool Load(std::string m_sDllPath, std::string sLogFilePath, int logLevel)
	{
		if (NULL != handler)
		{
			return true;  // already loaded
		}
        
        if(NULL != m_pIPVConnectivityInterface)
        {
            return true;
        }
        
		ipv_pointer pFn = NULL;
#ifdef _WIN32
		if (m_sDllPath.size() > 0)
		{
#if !(defined(WINDOWS_UNIVERSAL) || defined(WINDOWS_PHONE_8))
			handler = LoadLibraryA(m_sDllPath.c_str());
#endif

		}

		if (NULL == handler)
		{
#if !(defined(WINDOWS_UNIVERSAL) || defined(WINDOWS_PHONE_8))
			handler = LoadLibraryA(IPVisionLIB);
#endif

			if (NULL == handler)
			{
				return false;  // could not load DLL
			}
		}
		pFn = (ipv_pointer)GetProcAddress(handler, IPV_DLL_NAME);
        
        
#elif TARGET_OS_IPHONE
        
        pFn =  (ipv_pointer) IPVisionConnectivity;

#elif TARGET_IPHONE_SIMULATOR
        pFn =  (ipv_pointer) IPVisionConnectivity;
#else
        
		handler = dlopen(IPVisionLIB, RTLD_LAZY);
		if (!handler) {
			printf("CIPVConnectivityDLL::Load() Fail to Load.\n");
			//fprintf(stderr, "%s\n", dlerror());
			return false;
		}

		pFn = (ipv_pointer)dlsym(handler, IPV_DLL_NAME);

#endif

		if (NULL == pFn)
		{
			return false;  // could not load function from DLL
		}

		m_pIPVConnectivityInterface = (CIPVConnectivityInterface*)(pFn)(sLogFilePath.c_str(), logLevel);

		if (NULL == m_pIPVConnectivityInterface)
		{
			return false;  // could not execute function
		}

		return true;
	}


#ifdef _WIN32
	std::wstring StringToWideString(const std::string& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}
#endif

	std::string CopyString(CIPVStdString* pString)	
	{
		if (pString != NULL)
		{
			std::string s2(*pString);
			m_pIPVConnectivityInterface->DeleteString(pString);
			return s2;
		}
		return "";
	}

	bool SetUserName(const IPVLongType& lUserName)
	{
		return (m_pIPVConnectivityInterface) ? (m_pIPVConnectivityInterface->SetUserName(lUserName)) : false;
	}

};
#endif