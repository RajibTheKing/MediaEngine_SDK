//
//  VersionController.hpp
//  AudioVideoEngine
//
//  Created by Rajib Chandra Das on 4/23/16.
//  Copyright © 2016 ipvision. All rights reserved.
//

#ifndef IPV_VERSION_CONTROLLER_H
#define IPV_VERSION_CONTROLLER_H

#include <stdio.h>

namespace MediaSDK
{

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

} //namespace MediaSDK


#endif /* VersionController_hpp */
