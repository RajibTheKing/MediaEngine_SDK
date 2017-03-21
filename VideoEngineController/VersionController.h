//
//  VersionController.hpp
//  AudioVideoEngine
//
//  Created by Rajib Chandra Das on 4/23/16.
//  Copyright © 2016 ipvision. All rights reserved.
//

#ifndef VersionController_h
#define VersionController_h

#include <stdio.h>

class CVersionController
{
public:
    CVersionController();
    ~CVersionController();
    
    void SetCurrentCallVersion(int  uchVersion);
    int GetCurrentCallVersion();
    
    
    unsigned char GetOwnVersion();
    void SetOpponentVersion(int iVersion);
    int  GetOpponentVersion();

//    void SetOpponentVersionCompatibleFlag(bool bValue);
    bool GetOpponentVersionCompatibleFlag();
    
    bool IsFirstVideoPacetReceived();
    void NotifyFirstVideoPacetReceived();
    
    
private:
    
    unsigned char m_uchOwnVersion;
    
    int m_iOppVersion;
    int m_iCurrentCallVersion;
    bool m_bFirstVideoPacketReceivedFlag;
    
    bool m_bIsOpponentVersionDetectable;
    
    
    
    
};

#endif /* VersionController_hpp */
