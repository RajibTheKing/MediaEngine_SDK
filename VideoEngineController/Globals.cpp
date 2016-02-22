//
// Created by MD MAKSUD HOSSAIN on 1/1/16.
//

#include "Globals.h"
#include "ResendingBuffer.h"

CFPSController g_FPSController;
CResendingBuffer g_ResendBuffer;
PairMap g_timeInt;

bool g_bIsVersionDetectableOpponent = false;
unsigned char g_uchSendPacketVersion = 0;
int g_uchOpponentVersion = -1;