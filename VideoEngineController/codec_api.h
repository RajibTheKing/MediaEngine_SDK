
#ifndef _CODEC_API_H_
#define _CODEC_API_H_

#ifndef __cplusplus
#ifdef _MSC_VER
typedef unsigned char bool;
#else
#include <stdbool.h>
#endif
#endif

#include "codec_app_def.h"
#include "codec_def.h"

#if defined(_WIN32) || defined(__cdecl)
#define EXTAPI __cdecl
#else
#define EXTAPI
#endif

#ifdef __cplusplus

class ISVCEncoder 
{
 
public:
 
	virtual int EXTAPI Initialize (const SEncParamBase* pParam) = 0;

	virtual int EXTAPI InitializeExt (const SEncParamExt* pParam) = 0;

	virtual int EXTAPI GetDefaultParams (SEncParamExt* pParam) = 0;
 
	virtual int EXTAPI Uninitialize() = 0;

	virtual int EXTAPI EncodeFrame (const SSourcePicture* kpSrcPic, SFrameBSInfo* pBsInfo) = 0;

	virtual int EXTAPI EncodeParameterSets (SFrameBSInfo* pBsInfo) = 0;

	virtual int EXTAPI ForceIntraFrame (bool bIDR) = 0;

	virtual int EXTAPI SetOption (ENCODER_OPTION eOptionId, void* pOption) = 0;

	virtual int EXTAPI GetOption (ENCODER_OPTION eOptionId, void* pOption) = 0;

	virtual ~ISVCEncoder() {}
};

class ISVCDecoder 
{
 
public:

	virtual long EXTAPI Initialize (const SDecodingParam* pParam) = 0;

	virtual long EXTAPI Uninitialize() = 0;

	virtual DECODING_STATE EXTAPI DecodeFrame (const unsigned char* pSrc, const int iSrcLen, unsigned char** ppDst, int* pStride, int& iWidth, int& iHeight) = 0;

	virtual DECODING_STATE EXTAPI DecodeFrameNoDelay (const unsigned char* pSrc, const int iSrcLen, unsigned char** ppDst, SBufferInfo* pDstInfo) = 0;

	virtual DECODING_STATE EXTAPI DecodeFrame2 (const unsigned char* pSrc, const int iSrcLen, unsigned char** ppDst, SBufferInfo* pDstInfo) = 0;

	virtual DECODING_STATE EXTAPI DecodeParser (const unsigned char* pSrc, const int iSrcLen, SParserBsInfo* pDstInfo) = 0;

	virtual DECODING_STATE EXTAPI DecodeFrameEx (const unsigned char* pSrc, const int iSrcLen, unsigned char* pDst, int iDstStride, int& iDstLen, int& iWidth, int& iHeight, int& iColorFormat) = 0;

	virtual long EXTAPI SetOption (DECODER_OPTION eOptionId, void* pOption) = 0;

	virtual long EXTAPI GetOption (DECODER_OPTION eOptionId, void* pOption) = 0;

	virtual ~ISVCDecoder() {}
};


extern "C"
{

#else

	typedef struct ISVCEncoderVtbl ISVCEncoderVtbl;

	typedef const ISVCEncoderVtbl* ISVCEncoder;
	
	struct ISVCEncoderVtbl 
	{
		int (*Initialize) (ISVCEncoder*, const SEncParamBase* pParam);

		int (*InitializeExt) (ISVCEncoder*, const SEncParamExt* pParam);

		int (*GetDefaultParams) (ISVCEncoder*, SEncParamExt* pParam);

		int (*Uninitialize) (ISVCEncoder*);

		int (*EncodeFrame) (ISVCEncoder*, const SSourcePicture* kpSrcPic, SFrameBSInfo* pBsInfo);

		int (*EncodeParameterSets) (ISVCEncoder*, SFrameBSInfo* pBsInfo);

		int (*ForceIntraFrame) (ISVCEncoder*, bool bIDR);

		int (*SetOption) (ISVCEncoder*, ENCODER_OPTION eOptionId, void* pOption);

		int (*GetOption) (ISVCEncoder*, ENCODER_OPTION eOptionId, void* pOption);
	};

	typedef struct ISVCDecoderVtbl ISVCDecoderVtbl;

	typedef const ISVCDecoderVtbl* ISVCDecoder;
	
	struct ISVCDecoderVtbl 
	{
		long (*Initialize) (ISVCDecoder*, const SDecodingParam* pParam);

		long (*Uninitialize) (ISVCDecoder*);

		DECODING_STATE (*DecodeFrame) (ISVCDecoder*, const unsigned char* pSrc, const int iSrcLen, unsigned char** ppDst, int* pStride, int* iWidth, int* iHeight);

		DECODING_STATE (*DecodeFrameNoDelay) (ISVCDecoder*, const unsigned char* pSrc, const int iSrcLen, unsigned char** ppDst, SBufferInfo* pDstInfo);

		DECODING_STATE (*DecodeFrame2) (ISVCDecoder*, const unsigned char* pSrc, const int iSrcLen, unsigned char** ppDst, SBufferInfo* pDstInfo);

		DECODING_STATE (*DecodeParser) (ISVCDecoder*, const unsigned char* pSrc, const int iSrcLen, SParserBsInfo* pDstInfo);

		DECODING_STATE (*DecodeFrameEx) (ISVCDecoder*, const unsigned char* pSrc, const int iSrcLen, unsigned char* pDst, int iDstStride, int* iDstLen, int* iWidth, int* iHeight, int* iColorFormat);

		long (*SetOption) (ISVCDecoder*, DECODER_OPTION eOptionId, void* pOption);

		long (*GetOption) (ISVCDecoder*, DECODER_OPTION eOptionId, void* pOption);
	};

#endif

	typedef void (*WelsTraceCallback) (void* ctx, int level, const char* string);

	int  WelsCreateSVCEncoder (ISVCEncoder** ppEncoder);

	void WelsDestroySVCEncoder (ISVCEncoder* pEncoder);

	int WelsGetDecoderCapability (SDecoderCapability* pDecCapability);

	long WelsCreateDecoder (ISVCDecoder** ppDecoder);

	void WelsDestroyDecoder (ISVCDecoder* pDecoder);

	OpenH264Version WelsGetCodecVersion (void);

	void WelsGetCodecVersionEx (OpenH264Version *pVersion);

#ifdef __cplusplus

}

#endif

#endif
