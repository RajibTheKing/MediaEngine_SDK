
#ifdef USE_JNI
#ifndef __IPVConnectivity_DLL_JNI_LINKER_
#define __IPVConnectivity_DLL_JNI_LINKER_



#ifdef __ANDROID__
#include <jni.h>
#elif defined(__linux__) || defined(WIN32) || defined(WIN64)
#include "../ThirdPartyLibrary/JNI/jni.h"
#elif  defined(__APPLE__)
#include "JavaVM/jni.h"
#endif

#include "IPVConnectivityDLLCInterface.h"
#include "IPVConnectivityDLLLinker.h"

void notifyClientMethod(int eventType);
void notifyClientMethodForFriend(int eventType, IPVLongType lFriendName, int mediaType);
void notifyClientMethodWithReceivedBytes(int eventType, IPVLongType lFriendName, int mediaType, int dataLenth, unsigned char data[]);

#ifndef _Included_helloworld_Main
#define _Included_helloworld_Main
#ifdef __cplusplus
extern "C" {
#endif

	JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_init(JNIEnv *env, jobject obj, jlong lUserID, jstring sLogFileLocation, jint logLevel);

	JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_initializeLibrary(JNIEnv *env, jobject obj, jlong lUserID);

	JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_setAuthenticationServer(JNIEnv *env, jobject obj, jstring jAuthServerIP, jint iAuthServerPort, jstring jAppSessionId);

	JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_createSession(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia, jstring jRelayServerIP, jint iRelayServerPort);

	JNIEXPORT void JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_setRelayServerInformation(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia, jstring jRelayServerIP, jint iRelayServerPort);

	JNIEXPORT void JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_startP2PCall(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia, jboolean bCaller);

	JNIEXPORT jboolean JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_isConnectionTypeHostToHost(JNIEnv *env, jobject obj, jlong lFriendID, jint mediaType);

	JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_send(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia, jbyteArray data, jint iLen);

	JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_sendTo(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia, jbyteArray data, jint iLen, jstring jDestinationIP, jint iDestinationPort);
	
	JNIEXPORT jbyteArray JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_recv(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia, jint iLen);

	JNIEXPORT jstring JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_getSelectedIPAddress(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia);

	JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_getSelectedPort(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia);
	
	JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_closeSession(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia);
	
	JNIEXPORT void JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_release(JNIEnv *env, jobject obj);

	JNIEXPORT void JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_interfaceChanged(JNIEnv *env, jobject obj);

	JNIEXPORT void JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_setLogFileLocation(JNIEnv *env, jobject obj, jstring loc);

#ifdef __cplusplus
}
#endif
#endif

#endif 

#endif
