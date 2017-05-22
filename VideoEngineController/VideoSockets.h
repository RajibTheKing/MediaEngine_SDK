//
//  VideoSockets.h
//  TestCamera 
//
//  Created by Apple on 11/18/15.
//
//

#ifndef VideoSockets_h
#define VideoSockets_h

//#include "VideoCallProcessor.h"
#include <iostream>
#include <string>

namespace MediaSDK
{
	using namespace std;

	#define MAXBUFFER_SIZE 1024
	#define byte unsigned char
	#define LAST_PACKET_CODE 1000
	#define FIRST_PACKET_CODE 1000
	#define FULL_PACKET_CODE 10000
	#define MAX_SEND_BUFFER_SIZE 1000


	class CAudioCallSession;

	class VideoSockets
	{
    
	public:
		VideoSockets();
		~VideoSockets();
		static VideoSockets* GetInstance();
    
		void SetAudioCallSession(CAudioCallSession *pAudioCallSession);

		void StartDataReceiverThread();
		void StopDataReceiverThread();
		void SetUserID(long long lUserId);
		void InitializeSocket(string sActualServerIP, int sActualServerPort);
		void BindSocketToReceiveRemoteData();
		void PacketReceiverForVideoSendSocket();
		void PacketReceiverForVideoData();
		void PacketReceiver();
		void DataReceiverThread();
    

		void InitializeSocketForRemoteUser(string sRemoteIp);
		void InitializeServerSocketForRemoteUser(int &SocketFd, string sRemoteIp, int iRemotePort);
    
		void SendToServer(byte sendingBytePacket[], int length);
		void SendToServerWithPacketize(byte sendingBytePacket[], int length);
    
		void SendToVideoSocket(byte sendingBytePacket[], int length);
		void SendToVideoSendSocket(byte sendingBytePacket[], int length);
		void SendPacket(byte sendingBytePacket[], int length);
		void ProcessReceivedData(byte *recievedData, int length);
    
		int ByteArrayToIntegerConvert( byte* rawData, int stratPoint );
 
		long long m_lUserId;
		bool m_bDataReceiverThread;
    
		unsigned char m_ucaDatatoSendBuffer[MAXBUFFER_SIZE];
		unsigned char m_ucaDatatoReceiveBuffer[MAXBUFFER_SIZE];
		int m_iReceiveBufferIndex;
    
		int m_iPort;
		CAudioCallSession *m_pMyAudioCallSession;

		static void *createReciverThread(void *params);
	};

	static VideoSockets *m_pVideoSockets = NULL;

} //namespace MediaSDK


#endif /* VideoSockets_h */
