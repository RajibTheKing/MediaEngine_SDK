//
//  VersionController.cpp
//  AudioVideoEngine
//
//  Created by Rajib Chandra Das on 4/23/16.
//  Copyright © 2016 ipvision. All rights reserved.
//

#include "VersionController.h"
#include "Size.h"
#include "Tools.h"
//#include "PacketHeader.h"
#include "VideoHeader.h"

CVersionController::CVersionController()
{
    m_uchOwnVersion = VIDEO_VERSION_CODE;
    
    m_iOppVersion = -1;
    m_iCurrentCallVersion = -1;
    
    m_bIsOpponentVersionDetectable = true;
    m_bFirstVideoPacketReceivedFlag = false;
    
}

CVersionController::~CVersionController()
{
    
}



unsigned char CVersionController::GetOwnVersion()
{
    return m_uchOwnVersion;
}

bool CVersionController::IsFirstVideoPacetReceived(){
    return m_bFirstVideoPacketReceivedFlag;
}

void CVersionController::NotifyFirstVideoPacetReceived(){
    m_bFirstVideoPacketReceivedFlag = true;
}

void CVersionController::SetCurrentCallVersion(int  iVersion)
{
    m_iCurrentCallVersion = iVersion;
}
int  CVersionController::GetCurrentCallVersion()
{
    return m_iCurrentCallVersion;
}

void CVersionController::SetOpponentVersion(int iVersion)
{
    //VLOG("#SOV# ########################################SetOpp : " + Tools::IntegertoStringConvert((int)iVersion));
    m_iOppVersion = iVersion;
}

int  CVersionController::GetOpponentVersion()
{
    return m_iOppVersion;
}


//void CVersionController::SetOpponentVersionCompatibleFlag(bool bValue)
//{
//    m_bIsOpponentVersionDetectable = bValue;
//}

bool CVersionController::GetOpponentVersionCompatibleFlag()
{
    return m_bIsOpponentVersionDetectable;
}







