#ifndef __IPV_CONNECTIVITY_EVENT_INTERFACE_H_
#define __IPV_CONNECTIVITY_EVENT_INTERFACE_H_

#ifdef WIN32
typedef __int64 IPVLongType;
#else 
typedef long long IPVLongType;
#endif

void notifyClientMethodE(int eventType);

void notifyClientMethodForFriendE(int eventType, IPVLongType lFriendID, int mediaType);

void notifyClientMethodWithReceivedBytesE(int eventType, IPVLongType lFriendID, int mediaType, int dataLenth, unsigned char data[]);

#endif