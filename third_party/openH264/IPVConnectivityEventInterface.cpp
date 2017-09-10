#ifdef USE_JNI
#include "IPVConnectivityDLLJNILinker.h"
#else
#include "ObjectiveCInterFace.h"
#endif

void notifyClientMethodE(int eventType)
{
#ifndef USE_CPP
#ifdef USE_JNI
	notifyClientMethod(eventType);
#else
	notifyClientMethodIos(eventType);
#endif
#endif
}

void notifyClientMethodForFriendE(int eventType, IPVLongType lFriendID, int iMedia)
{
#ifndef USE_CPP
#ifdef USE_JNI
  	notifyClientMethodForFriend(eventType, lFriendID, iMedia);
#else
	notifyClientMethodForFriendIos(eventType, lFriendID, iMedia);
#endif
#endif
}

void notifyClientMethodWithReceivedBytesE(int eventType, IPVLongType lFriendID, int iMedia, int dataLenth, unsigned char data[])
{
#ifndef USE_CPP
#ifdef USE_JNI
    notifyClientMethodWithReceivedBytes(eventType, lFriendID, iMedia, dataLenth, data);
#else
	notifyClientMethodWithReceivedIos(eventType, lFriendID, iMedia, dataLenth, data);
#endif
#endif
}
