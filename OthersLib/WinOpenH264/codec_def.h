
#ifndef _CODEC_DEF_H_
#define _CODEC_DEF_H_

typedef enum 
{
	videoFormatRGB        = 1,             
	videoFormatRGBA       = 2,
	videoFormatRGB555     = 3,
	videoFormatRGB565     = 4,
	videoFormatBGR        = 5,
	videoFormatBGRA       = 6,
	videoFormatABGR       = 7,
	videoFormatARGB       = 8,

	videoFormatYUY2       = 20,           
	videoFormatYVYU       = 21,
	videoFormatUYVY       = 22,
	videoFormatI420       = 23,           
	videoFormatYV12       = 24,
	videoFormatInternal   = 25,           

	videoFormatNV12       = 26,           

	videoFormatVFlip      = 0x80000000

} EVideoFormatType;

typedef enum 
{
	videoFrameTypeInvalid,    
	videoFrameTypeIDR,        
	videoFrameTypeI,         
	videoFrameTypeP,         
	videoFrameTypeSkip,       
	videoFrameTypeIPMixed    

} EVideoFrameType;

typedef enum 
{
	cmResultSuccess,          
	cmInitParaError,         
	cmUnkonwReason,
	cmMallocMemeError,        
	cmInitExpected,          
	cmUnsupportedData

} CM_RETURN;

enum ENalUnitType 
{
	NAL_UNKNOWN     = 0,
	NAL_SLICE       = 1,
	NAL_SLICE_DPA   = 2,
	NAL_SLICE_DPB   = 3,
	NAL_SLICE_DPC   = 4,
	NAL_SLICE_IDR   = 5,    
	NAL_SEI         = 6,     
	NAL_SPS         = 7,
	NAL_PPS         = 8               
};

enum ENalPriority 
{
	NAL_PRIORITY_DISPOSABLE = 0,
	NAL_PRIORITY_LOW        = 1,
	NAL_PRIORITY_HIGH       = 2,
	NAL_PRIORITY_HIGHEST    = 3
};

#define IS_PARAMETER_SET_NAL(eNalRefIdc, eNalType) \
( (eNalRefIdc == NAL_PRIORITY_HIGHEST) && (eNalType == (NAL_SPS|NAL_PPS) || eNalType == NAL_SPS) )

#define IS_IDR_NAL(eNalRefIdc, eNalType) \
( (eNalRefIdc == NAL_PRIORITY_HIGHEST) && (eNalType == NAL_SLICE_IDR) )

#define FRAME_NUM_PARAM_SET     (-1)
#define FRAME_NUM_IDR           0

enum 
{
	DEBLOCKING_IDC_0 = 0,
	DEBLOCKING_IDC_1 = 1,
	DEBLOCKING_IDC_2 = 2
};

#define DEBLOCKING_OFFSET (6)
#define DEBLOCKING_OFFSET_MINUS (-6)

typedef unsigned short ERR_TOOL;

enum 
{
	ET_NONE = 0x00,          
	ET_IP_SCALE = 0x01,      
	ET_FMO = 0x02,            
	ET_IR_R1 = 0x04,         
	ET_IR_R2 = 0x08,          
	ET_IR_R3 = 0x10,         
	ET_FEC_HALF = 0x20,       
	ET_FEC_FULL = 0x40,     
	 ET_RFS = 0x80             
};

typedef struct SliceInformation 
{
	unsigned char* pBufferOfSlices;   
	int            iCodedSliceCount;  
	unsigned int*  pLengthOfSlices;   
	int            iFecType;          
	unsigned char  uiSliceIdx;        
	unsigned char  uiSliceCount;       
	char           iFrameIndex;       
	unsigned char  uiNalRefIdc;       
	unsigned char  uiNalType;         
	unsigned char
	uiContainingFinalNal;             

} SliceInfo, *PSliceInfo;

typedef struct
{
	int   iWidth;                  
	int   iHeight;                 
	int   iThresholdOfInitRate;    
	int   iThresholdOfMaxRate;     
	int   iThresholdOfMinRate;    
	int   iMinThresholdFrameRate;  
	int   iSkipFrameRate;          
	int   iSkipFrameStep;   

} SRateThresholds, *PRateThresholds;

typedef struct TagSysMemBuffer
{
	int iWidth;                  
	int iHeight;                  
	int iFormat;                   
	int iStride[2];                

} SSysMEMBuffer;

typedef struct TagBufferInfo 
{
	int iBufferStatus;             
	unsigned long long uiInBsTimeStamp;    
	unsigned long long uiOutYuvTimeStamp;     
  
	union
	{
		SSysMEMBuffer sSystemBuffer; 
	
	} UsrData;                    

} SBufferInfo;

static const char kiKeyNumMultiple[] =	{
											1, 1, 2, 4, 8, 16,
										};

#endif
