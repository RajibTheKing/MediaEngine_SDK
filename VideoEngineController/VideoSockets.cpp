//
//  VideoSockets.m
//  TestCamera 
//
//  Created by Apple on 11/18/15.
//
//

#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <sstream>
#include <queue>
#include <sys/time.h>
#include <thread>
#include "LogPrinter.h"

#include "VideoSockets.h"
#include "AudioCallSession.h"

struct sockaddr_in si_me, si_other, si_VideoSocket, si_VideoSendSocket;
struct sockaddr_in Server;

int ServerFd, s, i, slen = sizeof(si_other) , recv_len, s_VideoSocket, s_VideoSendSocket, sVideoSocketLen = sizeof(si_VideoSocket), sVideoSendSocketLen = sizeof(si_VideoSendSocket);

byte baDataReceiverBuffer[MAXBUFFER_SIZE];
byte baVideoReceiverBuffer[MAXBUFFER_SIZE];
byte baActualData[MAXBUFFER_SIZE];



VideoSockets::VideoSockets()
{
    m_bDataReceiverThread = false;
    printf("Inside VideoSockets Constructor");
    m_iReceiveBufferIndex = 0;
	m_pMyAudioCallSession = NULL;
}

VideoSockets::~VideoSockets()
{
    
}
VideoSockets* VideoSockets::GetInstance()
{
    if(m_pVideoSockets == NULL)
    {
        m_pVideoSockets = new VideoSockets();
    }
    return m_pVideoSockets;
}

void VideoSockets::SetAudioCallSession(CAudioCallSession *pAudioCallSession)
{
	m_pMyAudioCallSession = pAudioCallSession;
}

void VideoSockets::InitializeSocket(string sActualServerIP, int sActualServerPort)
{
    m_iPort = sActualServerPort;
    
    if ( (s_VideoSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("socket");
    }
    
    memset((char *) &si_VideoSocket, 0, sizeof(si_VideoSocket));
    si_VideoSocket.sin_family = AF_INET;
    si_VideoSocket.sin_port = htons(sActualServerPort);
    
    if (inet_aton(sActualServerIP.c_str() , &si_VideoSocket.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    
	StartDataReceiverThread();
    /*dispatch_queue_t PacketReceiverQ = dispatch_queue_create("PacketReceiverQ",DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(PacketReceiverQ, ^{
        //[self PacketReceiver];
        PacketReceiver();
    });
    
    dispatch_queue_t PacketReceiverForVideoDataQ = dispatch_queue_create("PacketReceiverForVideoDataQ",DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(PacketReceiverForVideoDataQ, ^{
        //[self PacketReceiverForVideoData];
        PacketReceiverForVideoData();
    });*/
    
    
    
}

void *VideoSockets::createReciverThread(void* params) {
	VideoSockets *pThis = (VideoSockets*)params;
	pThis->DataReceiverThread();
	return nullptr;
}
void VideoSockets::StartDataReceiverThread()
{
    if(m_bDataReceiverThread == true) return;
    
    m_bDataReceiverThread = true;

	std::thread myThread(createReciverThread, this);
	
	myThread.detach();
    
	/*
    dispatch_queue_t PacketReceiverQ = dispatch_queue_create("PacketReceiverQ",DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(PacketReceiverQ, ^{
        //[self PacketReceiver];
        DataReceiverThread();
    });
	*/
}
void VideoSockets::StopDataReceiverThread()
{
    m_bDataReceiverThread = false;
    
}
void VideoSockets::SetUserID(long long lUserId)
{
    m_lUserId = lUserId;
}

void  VideoSockets::BindSocketToReceiveRemoteData()
{
    if ( (s_VideoSendSocket =socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("socket");
    }
    
    
    memset((char *) &si_VideoSendSocket, 0, sizeof(si_VideoSendSocket));
    si_VideoSendSocket.sin_family = AF_INET;
    si_VideoSendSocket.sin_port = htons(32321);
    
    si_VideoSendSocket.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //bind socket to port
    if( bind(s_VideoSendSocket , (struct sockaddr*)&si_VideoSendSocket, sizeof(si_VideoSendSocket) ) == -1)
    {
        printf("bind\n");
    }
    
    
	/*
    dispatch_queue_t PacketReceiverForVideoSendSocketQ = dispatch_queue_create("PacketReceiverForVideoSendSocketQ",DISPATCH_QUEUE_SERIAL);
    dispatch_async(PacketReceiverForVideoSendSocketQ, ^{
        //[self PacketReceiverForVideoSendSocket];
        PacketReceiverForVideoSendSocket();
    });*/
}

void VideoSockets::SendToVideoSocket(byte sendingBytePacket[], int length)
{
    int iRet;
    iRet = sendto(s_VideoSocket, sendingBytePacket, length, 0, (struct sockaddr*) &si_VideoSocket, sizeof(si_VideoSocket));
    
    printf("SendPacketVideoSocket, iRet = %d\n", iRet);
    
}

void VideoSockets::SendToVideoSendSocket(byte sendingBytePacket[], int length)
{
    int iRet;
    printf("First byte of sending video data = %d\n", (int)sendingBytePacket[0]);
    iRet = sendto(s_VideoSendSocket, sendingBytePacket, length, 0, (struct sockaddr*) &si_VideoSendSocket, sizeof(si_VideoSendSocket));
    
    printf("SendPacketVideoSocket, iRet = %d\n", iRet);
    
}


void VideoSockets::PacketReceiverForVideoSendSocket()
{
    printf("Rajib_Check: Inside PacketReceiveerforVideoSocket called\n");
    struct sockaddr_in sss_other;
    int ssslen = sizeof(si_other);
    while(true)
    {
        printf("Waiting for data..., PacketReceiverForVideoSendSocket");
        fflush(stdout);
        int iDataLen;
        //try to receive some data, this is a blocking call
        
        if ((iDataLen = recvfrom(s_VideoSendSocket, baVideoReceiverBuffer, MAXBUFFER_SIZE, 0, (struct sockaddr *) &sss_other, (socklen_t*)&ssslen)) == -1)
        {
            printf("ERROR: Packet receive Failed\n");
            continue;
        }
        
        printf("Data VideoSendSocket type: %d, iRet = %d\n" , (int)baVideoReceiverBuffer[0], iDataLen);
        
        int iPacketType = (int)baVideoReceiverBuffer[0];
        if(iPacketType == 33 && iDataLen !=-1)
        {
            /*
            //MARK: receiving video data.
            NSData *receivedData = [[NSData alloc] initWithBytes:baVideoReceiverBuffer+1 length:iDataLen-1];
            [[RingCallAudioManager sharedInstance] processReceivedRTPPacket:receivedData];
            */
            //long long lUser = [[VideoCallProcessor GetInstance] GetUserId];
            //m_pVideoAPI->PushPacketForDecodingV(m_lUserId, baVideoReceiverBuffer+1, iDataLen-1);
            
            //int availableBytes = 0;
            //TPCircularBufferProduceBytes([[RingCallAudioManager sharedInstance]., shortArray, (availableBytes));
            
            //m_pVideoAPI->PushPacketForDecoding(m_lUserId, baVideoReceiverBuffer+1, iDataLen-1);
            
            /*printf("VideoTeamCheck: AUDIO PACKET Found, iLen = %d\n", iDataLen);
            
            NSData *receivedData = [[NSData alloc] initWithBytes:baVideoReceiverBuffer+1 length:iDataLen-1];
            [[RingCallAudioManager sharedInstance] processReceivedRTPPacket:receivedData];*/
            
        }
        else if(iPacketType == 43 && iDataLen !=-1)
        {
            
            printf("VideoTeamCheck: AUDIO PACKET Found, iLen = %d\n", iDataLen);
            
            // NSData *receivedData = [[NSData alloc] initWithBytes:baVideoReceiverBuffer+1 length:iDataLen-1];
            // [[RingCallAudioManager sharedInstance] processReceivedRTPPacket:receivedData];
            
            
        }
        else
        {
            /*
            printf("VideoTeamCheck: AUDIO PACKET Found, iLen = %d\n", iDataLen);
            
            NSData *receivedData = [[NSData alloc] initWithBytes:baVideoReceiverBuffer length:iDataLen];
            [[RingCallAudioManager sharedInstance] processReceivedRTPPacket:receivedData];
            
            
            
            //WriteToFileV(byteData, len);
            
            //CVideoAPI::GetInstance()->PushAudioForDecoding(200, byteData, len);
             
             */
        }
        
    }
    
}

void VideoSockets::DataReceiverThread()
{
    byte testServer[] = "HudaiEktaMessage";
    
    SendToServer(testServer, 16);
    
    int startPrefix = 0;
    
    while(m_bDataReceiverThread)
    {
        fflush(stdout);
        
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s_VideoSocket, baDataReceiverBuffer, MAXBUFFER_SIZE, 0, (struct sockaddr *) &si_VideoSocket, (socklen_t*)&sVideoSocketLen)) == -1)
        {
            PRT("Data recv error with code = %d\n", recv_len);
            continue;
        }
		PRT("RECEIVED LEN DA %d", recv_len);
		m_pMyAudioCallSession->m_AudioReceivedBuffer.EnQueue(baDataReceiverBuffer + 1, recv_len - 1);
        // int iPacketType = (int)baDataReceiverBuffer[0];
		
        
        //if(iPacketType == FULL_PACKET_CODE)
        //{
        //    //CVideoAPI::GetInstance()->PushPacketForDecoding(200, 3, ENTITY_TYPE_VIEWER, baDataReceiverBuffer+1, recv_len-1);
        //    ProcessReceivedData(baDataReceiverBuffer+1, recv_len - 1);
        //    m_iReceiveBufferIndex = 0;
        //}
        //else
        //{
        //    if(iPacketType == LAST_PACKET_CODE)
        //    {
        //        memcpy(m_ucaDatatoReceiveBuffer+m_iReceiveBufferIndex, baDataReceiverBuffer+1, recv_len-1);
        //        m_iReceiveBufferIndex+=(recv_len-1);
        //        //CVideoAPI::GetInstance()->PushPacketForDecoding(200, 3, ENTITY_TYPE_VIEWER, m_ucaDatatoReceiveBuffer, m_iReceiveBufferIndex);
        //        ProcessReceivedData(m_ucaDatatoReceiveBuffer, m_iReceiveBufferIndex);
        //        m_iReceiveBufferIndex=0;
        //    }
        //    else
        //    {
        //        memcpy(m_ucaDatatoReceiveBuffer+m_iReceiveBufferIndex, baDataReceiverBuffer+1, recv_len-1);
        //        m_iReceiveBufferIndex+=(recv_len-1);
        //    }
        //}
        
        /*
         if(iPacketType == 39)
            CVideoAPI::GetInstance()->PushPacketForDecoding(200, MEDIA_TYPE_VIDEO, ENTITY_TYPE_CALLER, baDataReceiverBuffer, recv_len);
        else
            CVideoAPI::GetInstance()->PushPacketForDecoding(200, MEDIA_TYPE_AUDIO, ENTITY_TYPE_CALLER, baDataReceiverBuffer, recv_len);
        */
        
        

    }
}

void VideoSockets::PacketReceiverForVideoData()
{
    
    while(true)
    {
        printf("Waiting for data, PacketReceiverForVideoData\n");
        fflush(stdout);
        
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s_VideoSocket, baDataReceiverBuffer, MAXBUFFER_SIZE, 0, (struct sockaddr *) &si_VideoSocket, (socklen_t*)&sVideoSocketLen)) == -1)
        {
            printf("recvfrom()");
        }
        
        //print details of the client/peer and the data received
        printf("Inside VIdeo Socket Received packet from %s:%d\n", inet_ntoa(si_VideoSocket.sin_addr), ntohs(si_VideoSocket.sin_port));
        printf("Data Inside VideoSocket: %d\n" , (int)baDataReceiverBuffer[0]);
        
        
        //if (recv_len != -1)
        //mySignalReceiver->SignalingPacketProcess((byte*)buf, recv_len);
        
        int iPacketType = (int)baDataReceiverBuffer[0];
        if(iPacketType == 31 )
        {
            cout<<"VIDEO_SERVER_REGISTATION_RESPONSE Found"<<endl;
            //ClientConstents.FRIEND_PORT = ByteArrayToIntegerConvert(receivedMessage, ++start);
            
            int start = 1;
            int iFriendPort = ByteArrayToIntegerConvert((byte*)baDataReceiverBuffer, ++start);
            
            
            //cout<<"VideoSocket:: VideoAPI->SetRelayServerInformation --> "<<"lUser = "<<[[VideoCallProcessor GetInstance] GetFriendId]<<", g_serverip  = "<<g_sServerIP<<", iFriendPort = "<<iFriendPort<<endl;
            //CVideoAPI::GetInstance()->SetRelayServerInformation(200, (int)2/*Audio*/,  "38.127.68.60", 60001);
            //CVideoAPI::GetInstance()->SetRelayServerInformation(200, (int)1/*Audio*/,  "38.127.68.60", 60001);
            
            /*
             if ( (s_VideoSendSocket =socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
             {
             printf("socket");
             }
             
             
             InitializeServerSocketForRemoteUser(s_VideoSendSocket, g_sServerIP, iFriendPort);
             
             dispatch_queue_t PacketReceiverForVideoSendSocketQ = dispatch_queue_create("PacketReceiverForVideoSendSocketQ",DISPATCH_QUEUE_SERIAL);
             dispatch_async(PacketReceiverForVideoSendSocketQ, ^{
             [self PacketReceiverForVideoSendSocket];
             });
             */
            
            
            /*
             
             bStartVideoSending = true;
             [g_pVideoCallProcessor StartAllThreads];
             //[self StartCameraSession];
             [session startRunning];
             
             */
            
        }
        else
        {
            cout<<"Packet Type Not Found: Invalid Data"<<endl;
        }
    }
}



void VideoSockets::PacketReceiver()
{
    
    while(true)
    {
        printf("Waiting for data...");
        fflush(stdout);
        
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, baDataReceiverBuffer, 1024, 0, (struct sockaddr *) &si_other, (socklen_t*)&slen)) == -1)
        {
            printf("recvfrom()");
        }
        
        //print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        printf("Data: %d\n" , (int)baDataReceiverBuffer[0]);
        
        
        int iPacketType = (int)baDataReceiverBuffer[0];
    }
}


void VideoSockets::InitializeSocketForRemoteUser(string sRemoteIp)
{
    if ( (ServerFd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("socket");
    }
    
    memset((char *) &Server, 0, sizeof(Server));
    Server.sin_family = AF_INET;
    Server.sin_port = htons(32321);
    
    //if (inet_aton("192.168.57.152" , &Server.sin_addr) == 0)
    if (inet_aton(sRemoteIp.c_str() , &Server.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
}


void VideoSockets::InitializeServerSocketForRemoteUser(int &SocketFd, string sRemoteIp, int iRemotePort)
{
    
    if ( (ServerFd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("socket");
    }
    
    memset((char *) &Server, 0, sizeof(Server));
    Server.sin_family = AF_INET;
    Server.sin_port = htons(iRemotePort);
    
    if (inet_aton(sRemoteIp.c_str() , &Server.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    SocketFd = ServerFd;
}

void VideoSockets::SendToServer(byte sendingBytePacket[], int length)
{
    int iRet;
    //s_VideoSendSocket
    
    iRet = sendto(s_VideoSocket, sendingBytePacket, length, MAXBUFFER_SIZE, (struct sockaddr*) &si_VideoSocket, sizeof(si_VideoSocket));
    
    //iRet = send(s_VideoSocket, sendingBytePacket, length, 0);
    
    //iRet = sendto(s_VideoSendSocket, sendingBytePacket, length, 0, (struct sockaddr*) &Server, sizeof(Server));
    
	PRT("-->SendPacketVideoSocket, iRet = %d, errno = %d\n", iRet, errno);
    
//    fprintf(stderr, "socket() failed: %s\n", strerror(errno));
    
    
}


byte g_dataToSend[8005];

void VideoSockets::SendToServerWithPacketize(byte sendingBytePacket[], int length)
{
    int iRet;
    if(length < 9216)
    {
        memcpy(m_ucaDatatoSendBuffer+1, sendingBytePacket, length);
        m_ucaDatatoSendBuffer[0] = FULL_PACKET_CODE; //FullPacket
        iRet = sendto(s_VideoSocket, m_ucaDatatoSendBuffer, length+1, MAXBUFFER_SIZE, (struct sockaddr*) &si_VideoSocket, sizeof(si_VideoSocket));
        printf("-->SendPacketVideoSocket, iRet = %d, errno = %d\n", iRet, errno);
    }
    else
    {
        int cnt = 1;
        int indx = 0;
        while(indx<length)
        {
            int dataToSendSize = min(MAX_SEND_BUFFER_SIZE, length-indx);
            memcpy(m_ucaDatatoSendBuffer+1, sendingBytePacket+indx, dataToSendSize);
            m_ucaDatatoSendBuffer[0] = cnt++;
            indx+=dataToSendSize;
            if(indx>=length)
            {
                m_ucaDatatoSendBuffer[0] = LAST_PACKET_CODE;
            }
            
            iRet = sendto(s_VideoSocket, m_ucaDatatoSendBuffer, dataToSendSize+1, MAXBUFFER_SIZE, (struct sockaddr*) &si_VideoSocket, sizeof(si_VideoSocket));
            printf("-->SendPacketVideoSocket, iRet = %d, errno = %d\n", iRet, errno);
        }
    }
    
    
}

void VideoSockets::SendPacket(byte sendingBytePacket[], int length)
{
    //now reply the client with the same data
    int iRet;
    iRet = sendto(s, sendingBytePacket, length, 0, (struct sockaddr*) &si_other, sizeof(si_other));
    
    PRT("SendPacket, iRet = %d\n", iRet);
    
}
int VideoSockets::ByteArrayToIntegerConvert( byte* rawData, int stratPoint )
{
    int TotalDataLen = 0;
    
    TotalDataLen += (rawData[stratPoint++] & 0xFF) << 24;
    TotalDataLen += (rawData[stratPoint++] & 0xFF) << 16;
    TotalDataLen += (rawData[stratPoint++] & 0xFF) << 8;
    TotalDataLen += (rawData[stratPoint++] & 0xFF);
    
    return TotalDataLen;
}
void VideoSockets::ProcessReceivedData(byte *recievedData, int length)
{
    if(m_iPort == 60001)
    {
        //CVideoAPI::GetInstance()->PushPacketForDecoding(200, 4, ENTITY_TYPE_PUBLISHER, recievedData, length);
    }
    else
    {
        //CVideoAPI::GetInstance()->PushPacketForDecoding(200, 3, ENTITY_TYPE_VIEWER, recievedData, length);
    }
    
}
