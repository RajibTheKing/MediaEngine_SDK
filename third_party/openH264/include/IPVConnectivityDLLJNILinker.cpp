#ifdef USE_JNI
#include "IPVConnectivityDLLJNILinker.h"

static JavaVM *jvm = NULL;
static jobject callback = NULL;

unsigned char* toConstChar(jbyteArray array, JNIEnv *env)
{
	int len = env->GetArrayLength(array);
	unsigned char* buf = new unsigned char[len];
	env->GetByteArrayRegion(array, 0, len, reinterpret_cast<jbyte*>(buf));
	return buf;
}

JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_init(JNIEnv *env, jobject obj, jlong lUserID, jstring sLogFilePath, jint logLevel)
{
	env->GetJavaVM(&jvm);
	callback = env->NewGlobalRef(obj);

	const char *cLogPath = env->GetStringUTFChars(sLogFilePath, JNI_FALSE);

	jint ret =  ipv_Init(lUserID, cLogPath, logLevel);

	return ret;
}

JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_initializeLibrary(JNIEnv *env, jobject obj, jlong lUserID)
{
	env->GetJavaVM(&jvm);
	callback = env->NewGlobalRef(obj);

	jint ret = ipv_InitializeLibrary(lUserID);
	return ret;
}

JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_setAuthenticationServer(JNIEnv *env, jobject obj, jstring jAuthServerIP, jint iAuthServerPort, jstring jAppSessionId)
{
	const char *authServerIPLineChar = env->GetStringUTFChars(jAuthServerIP, NULL);
    const char *appSessionIdLineChar = env->GetStringUTFChars(jAppSessionId, NULL);

	jint iRet = ipv_SetAuthenticationServer(authServerIPLineChar, iAuthServerPort, appSessionIdLineChar);

	return iRet;
}

JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_createSession(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia, jstring jRelayServerIP, jint iRelayServerPort)
{
	const char *relayServerIPLineChar = env->GetStringUTFChars(jRelayServerIP, NULL);

	jint iRet = ipv_CreateSession(lFriendID, (MediaType)jMedia, relayServerIPLineChar, iRelayServerPort);

	return iRet;
}

JNIEXPORT void JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_setRelayServerInformation(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia, jstring jRelayServerIP, jint iRelayServerPort)
{
	const char *relayServerIPLineChar = env->GetStringUTFChars(jRelayServerIP, NULL);

	ipv_SetRelayServerInformation(lFriendID, (MediaType)jMedia, relayServerIPLineChar, iRelayServerPort);
}

JNIEXPORT void JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_startP2PCall(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia, jboolean bCaller)
{
	ipv_StartP2PCall(lFriendID, (MediaType)jMedia, (bCaller == (jboolean)true) ? 1 : 0);
}

JNIEXPORT jboolean JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_isConnectionTypeHostToHost(JNIEnv *env, jobject obj, jlong lFriendID, jint mediaType)
{
	return ipv_IsConnectionTypeHostToHost(lFriendID, (MediaType)mediaType);
}

JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_send(JNIEnv *env, jobject obj, jlong lFriendID, jint mediaType, jbyteArray data, jint iLen)
{
	unsigned char *dataChar = toConstChar(data, env);

	return ipv_Send(lFriendID, (MediaType)mediaType, dataChar, iLen);
}

JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_sendTo(JNIEnv *env, jobject obj, jlong lFriendID, jint mediaType, jbyteArray data, jint iLen, jstring jDestinationIP, jint iDestinationPort)
{
	unsigned char *dataChar = toConstChar(data, env);
	const char *dstAddressLineChar = env->GetStringUTFChars(jDestinationIP, NULL);
	
	return ipv_SendTo(lFriendID, (MediaType)mediaType, dataChar, iLen, dstAddressLineChar, iDestinationPort);
}

JNIEXPORT jbyteArray JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_recv(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia, jint iLen)
{
	unsigned char* dataChar = new unsigned char[4096];

	int returnValue = ipv_Recv(lFriendID, (MediaType)jMedia, dataChar, iLen);

	if (returnValue == -1)
		return NULL;

	jbyteArray byteArrayData = env->NewByteArray(returnValue);

	env->SetByteArrayRegion(byteArrayData, 0, returnValue, reinterpret_cast<jbyte*>(dataChar));

	delete[] dataChar;

	return byteArrayData;
}

JNIEXPORT jstring JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_getSelectedIPAddress(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia)
{
	const char* dataChar = ipv_GetSelectedIPAddress(lFriendID, (MediaType)jMedia);

	return (env)->NewStringUTF(dataChar);
}

JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_getSelectedPort(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia)
{
	return ipv_GetSelectedPort(lFriendID, (MediaType)jMedia);
}

JNIEXPORT jint JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_closeSession(JNIEnv *env, jobject obj, jlong lFriendID, jint jMedia)
{
	return ipv_CloseSession(lFriendID, (MediaType)jMedia);
}

JNIEXPORT void JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_release(JNIEnv *env, jobject obj)
{
	ipv_Release();
}

JNIEXPORT void JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_interfaceChanged(JNIEnv *env, jobject obj)
{
	ipv_InterfaceChanged();
}


JNIEXPORT void JNICALL Java_com_ringid_ringidcall_ConnectivityEngine_setLogFileLocation(JNIEnv *env, jobject obj, jstring loc)
{
	const char* cLocation = env->GetStringUTFChars(loc, JNI_FALSE);
	ipv_SetLogFileLocation(cLocation);
}

void notifyClientMethod(int eventType)
{
	if (jvm == NULL || callback == NULL)
		return;

	JNIEnv *env = NULL;
	jint res;

#ifndef ANDROID
	res = jvm->AttachCurrentThread((void **)&env, NULL);
#else
	res = (jvm)->AttachCurrentThread(&env, NULL);
#endif

	if (res >= 0)
	{
		jclass cls = env->GetObjectClass(callback);

		jmethodID mid = env->GetMethodID(cls, "notifyClientMethod", "(I)V");

		env->CallVoidMethod(callback, mid, eventType);

		jvm->DetachCurrentThread();
	}
}

void notifyClientMethodForFriend(int eventType, IPVLongType lFriendName, int mediaType)
{
	if (jvm == NULL || callback == NULL)
		return;

	JNIEnv *env = NULL;
	jint res;

#ifndef ANDROID
	res = jvm->AttachCurrentThread((void **)&env, NULL);
#else
	res = (jvm)->AttachCurrentThread(&env, NULL);
#endif

	if (res >= 0)
	{
		jclass cls = env->GetObjectClass(callback);

		jmethodID mid = env->GetMethodID(cls, "notifyClientMethodForFriend", "(JII)V");

		env->CallVoidMethod(callback, mid, lFriendName, mediaType, eventType);

		jvm->DetachCurrentThread();
	}
}

void notifyClientMethodWithReceivedBytes(int eventType, IPVLongType lFriendName, int mediaType, int dataLenth, unsigned char data[])
{
	if (jvm == NULL || callback == NULL)
		return;

	JNIEnv *env = NULL;
	jint res;

#ifndef ANDROID
	res = jvm->AttachCurrentThread((void **)&env, NULL);
#else
	res = (jvm)->AttachCurrentThread(&env, NULL);
#endif

	if (res >= 0)
	{
		jclass cls = env->GetObjectClass(callback);

		jmethodID mid = env->GetMethodID(cls, "notifyClientMethodWithReceivedBytes", "(JIII[B)V");

		jbyteArray	retArray = env->NewByteArray(dataLenth);
		void *temp = env->GetPrimitiveArrayCritical((jarray)retArray, 0);

		memcpy(temp, data, dataLenth);

		env->ReleasePrimitiveArrayCritical(retArray, temp, 0);

		env->CallVoidMethod(callback, mid, lFriendName, mediaType, eventType, dataLenth, retArray);

		jvm->DetachCurrentThread();
	}
}

#endif


