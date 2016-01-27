//
// Created by MD MAKSUD HOSSAIN on 1/1/16.
//

#include "Globals.h"

CFPSController g_FPSController;
PairMap g_timeInt;
bool g_bIsVersionDetectableOpponent = false;
//unsigned char g_uchSendPacketVersion = 0;
unsigned char g_uchSendPacketVersion = VIDEO_VERSION_CODE;
int g_uchOpponentVersion = -1;