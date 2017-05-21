
#ifndef IPV_ENCODER_LIST_HANDLER_H
#define IPV_ENCODER_LIST_HANDLER_H

#include <stdio.h>
#include <string>
#include <map>

#include "VideoEncoder.h"
#include "SmartPointer.h"
#include "CommonTypes.h"

using namespace std;

class CVideoEncoderListHandler
{

public:

	CVideoEncoderListHandler();
	~CVideoEncoderListHandler();

	void AddToVideoEncoderList(long long llFriendID, CVideoEncoder* pcMedia);
	CVideoEncoder* GetFromVideoEncoderList(long long llFriendID);
	CVideoEncoder* GetFromVideoEncoderListinIndex(int iIndex);
	bool RemoveFromVideoEncoderList(long long llFriendID);
	int SizeOfVideoEncoderList();
	bool IsVideoEncoderExist(int nVideoHeight, int nVideoWidth);
	bool IsVideoEncoderExist(long long llFriendID);
	void ClearAllFromVideoEncoderList();
	void ResetAllInVideoEncoderList();

private:

	std::map<long long, CVideoEncoder*> m_mVideoEncoderList;

protected:

	SmartPointer<CLockHandler> m_pVideoEncoderListMutex;
};

#endif