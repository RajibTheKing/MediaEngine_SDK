
#ifndef IPV_ENCODER_LIST_HANDLER_H
#define IPV_ENCODER_LIST_HANDLER_H

#include <stdio.h>
#include <string>
#include <map>

#include "VideoEncoder.h"
#include "SmartPointer.h"
#include "LockHandler.h"

using namespace std;

class CVideoEncoderListHandler
{

public:

	CVideoEncoderListHandler();
	~CVideoEncoderListHandler();

	void AddToVideoEncoderList(LongLong lFriendID, CVideoEncoder* media);
	CVideoEncoder* GetFromVideoEncoderList(LongLong lFriendID);
	CVideoEncoder* GetFromVideoEncoderListinIndex(int index);
	bool RemoveFromVideoEncoderList(LongLong lFriendID);
	int SizeOfVideoEncoderList();
	bool IsVideoEncoderExist(int iVideoHeight, int iVideoWidth);
	bool IsVideoEncoderExist(LongLong lFriendID);
	void ClearAllFromVideoEncoderList();
	void ResetAllInVideoEncoderList();

private:

	std::map<LongLong, CVideoEncoder*> VideoEncoderList;

protected:

	SmartPointer<CLockHandler> m_pVideoEncoderListMutex;
};

#endif