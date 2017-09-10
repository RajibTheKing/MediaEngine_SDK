//
//  ObjectiveCInterFace.h
//  ringID
//
//  Created by md zulfiker ali  on 7/1/15.
//  Copyright (c) 2015 IPVision Canada Inc. All rights reserved.
//

#ifndef ringID_ObjectiveCInterFace_h
#define ringID_ObjectiveCInterFace_h

#ifdef WIN32
typedef __int64 IPVLongType;
#else
typedef long long IPVLongType;
#endif


void notifyClientMethodIos(int eventType);
void notifyClientMethodForFriendIos(int eventType, IPVLongType friendName, int iMedia);
void notifyClientMethodWithReceivedIos(int eventType, IPVLongType friendName, int iMedia, int dataLenth, unsigned char data[]);


#endif
