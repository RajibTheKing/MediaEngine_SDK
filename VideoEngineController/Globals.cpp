//
// Created by MD MAKSUD HOSSAIN on 1/1/16.
//

#include "Globals.h"

CFPSController g_FPSController;
PairMap g_timeInt;

#ifdef FIRST_BUILD_COMPATIBLE
bool g_bIsVersionDetectableOpponent = false;
unsigned char g_uchSendPacketVersion = 0;
#else
bool g_bIsVersionDetectableOpponent = true;
unsigned char g_uchSendPacketVersion = 1;
#endif
int g_uchOpponentVersion = -1;