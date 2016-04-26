
#include "VideoEncodingThread.h"
#include "Globals.h"
#include "LogPrinter.h"
#include "VideoCallSession.h"

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
#include <dispatch/dispatch.h>
#endif

extern CFPSController g_FPSController;

CVideoEncodingThread::CVideoEncodingThread(LongLong llFriendID, CEncodingBuffer *pEncodingBuffer, BitRateController *pBitRateController, CColorConverter *pColorConverter, CVideoEncoder *pVideoEncoder, CEncodedFramePacketizer *pEncodedFramePacketizer, CVideoCallSession *pVideoCallSession, bool bIsCheckCall ) :

m_iFrameNumber(0),
m_llFriendID(llFriendID),
m_pEncodingBuffer(pEncodingBuffer),
m_pBitRateController(pBitRateController),
m_pColorConverter(pColorConverter),
m_pVideoEncoder(pVideoEncoder),
m_pEncodedFramePacketizer(pEncodedFramePacketizer),
mt_nTotalEncodingTimePerFrameRate(0),
m_bIsThisThreadStarted(false),
m_FPS_TimeDiff(0),
m_FpsCounter(0)
{
    m_pVideoCallSession = pVideoCallSession;
    m_bIsCheckCall = bIsCheckCall;
    
    if(m_bIsCheckCall == true)
     {
         for(int k=0;k<3;k++)
         {
             memset(m_ucaDummmyFrame[k], 0, sizeof(m_ucaDummmyFrame[k]));
             
             for(int i=0;i<this->m_pColorConverter->GetHeight();i++)
             {
                 int color = rand()%255;
                 for(int j = 0; j < this->m_pColorConverter->GetWidth(); j ++)
                 {
                     m_ucaDummmyFrame[k][i * this->m_pColorConverter->GetHeight() + j ] = color;
                 }
                 
             }
         }
         
     }
    
    
}

CVideoEncodingThread::~CVideoEncodingThread()
{

}

void CVideoEncodingThread::ResetVideoEncodingThread(BitRateController *pBitRateController)
{
    m_iFrameNumber = 0;
    m_pBitRateController = pBitRateController;
    
    
}
void CVideoEncodingThread::StopEncodingThread()
{
	//if (pInternalThread.get())
	{

		bEncodingThreadRunning = false;

		while (!bEncodingThreadClosed)
		{
			m_Tools.SOSleep(5);
		}
	}

	//pInternalThread.reset();
}

void CVideoEncodingThread::StartEncodingThread()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::StartEncodingThread called");

	if (pEncodingThread.get())
	{
		pEncodingThread.reset();
		
		return;
	}
	
	bEncodingThreadRunning = true;
	bEncodingThreadClosed = false;

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

	dispatch_queue_t EncodeThreadQ = dispatch_queue_create("EncodeThreadQ", DISPATCH_QUEUE_CONCURRENT);
	dispatch_async(EncodeThreadQ, ^{
		this->EncodingThreadProcedure();
	});

#else

	std::thread myThread(CreateVideoEncodingThread, this);
	myThread.detach();

#endif

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::StartEncodingThread Encoding Thread started");

	return;
}

void *CVideoEncodingThread::CreateVideoEncodingThread(void* param)
{
	CVideoEncodingThread *pThis = (CVideoEncodingThread*)param;
	pThis->EncodingThreadProcedure();

	return NULL;
}

void CVideoEncodingThread::SetOrientationType(int nOrientationType)
{
	m_nOrientationType = nOrientationType;
}

bool CVideoEncodingThread::IsThreadStarted()
{
	return m_bIsThisThreadStarted;
}

long long g_PrevEncodeTime = 0;

void CVideoEncodingThread::EncodingThreadProcedure()
{
	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::EncodingThreadProcedure() started EncodingThreadProcedure method");

	Tools toolsObject;
	int nEncodingFrameSize, nENCODEDFrameSize, nCaptureTimeDifference;
	long long llCalculatingTime;
	int sumOfEncodingTimediff = 0;
	int sumOfZeroLengthEncodingTimediff = 0;
	int countZeroLengthFrame = 0;

	/*for(int i = 0; i < 200; i++)
	{
		if (m_pBitRateController->IsNetworkTypeMiniPacketReceived())
		{
			CLogPrinter_WriteSpecific5(CLogPrinter::INFO, "CVideoEncodingThread::EncodingThreadProcedure() m_pBitRateController->m_iNetworkType after waiting = " + toolsObject.IntegertoStringConvert(m_pBitRateController->GetOpponentNetworkType()));
			break;
		}

		toolsObject.SOSleep(10);
	}*/

	m_pBitRateController->SetInitialBitrate();

	while (bEncodingThreadRunning)
	{
		CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::EncodingThreadProcedure() RUNNING EncodingThreadProcedure method");

		m_bIsThisThreadStarted = true;
        
        //printf("TheVersion--> CurrentCallVersion = %d\n", m_pVideoCallSession->GetVersionController()->GetCurrentCallVersion());
        if(m_pVideoCallSession->GetVersionController()->GetCurrentCallVersion() == -1)
        {
            
           m_pEncodedFramePacketizer->Packetize(m_llFriendID, m_ucaEncodedFrame, PACKET_HEADER_LENGTH_NO_VERSION + 2, /*m_iFrameNumber*/0, /*nCaptureTimeDifference*/0, true);
            toolsObject.SOSleep(20);
            continue;
        }
        
		if (m_pEncodingBuffer->GetQueueSize() == 0)
		{
			//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, " fahad Encode time buffer size 0");
			toolsObject.SOSleep(10);
		}
		else
		{
            
			int timeDiff;

			//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, " fahad Encode time ");

			nEncodingFrameSize = m_pEncodingBuffer->DeQueue(m_ucaEncodingFrame, timeDiff, nCaptureTimeDifference);
            
            if(g_PrevEncodeTime!=0)
                m_CalculateEncodingTimeDiff.UpdateData(m_Tools.CurrentTimestamp() - g_PrevEncodeTime);
            //printf("TheVampireEngg --> EncodingTime Diff = %lld, Average = %lf\n", m_Tools.CurrentTimestamp() - g_PrevEncodeTime, m_CalculateEncodingTimeDiff.GetAverage());
            g_PrevEncodeTime = m_Tools.CurrentTimestamp();
            
            
			CLogPrinter_WriteLog(CLogPrinter::INFO, QUEUE_TIME_LOG ," &*&*&* m_pEncodingBuffer ->" + toolsObject.IntegertoStringConvert(timeDiff));

            
			if (!g_FPSController.IsProcessableFrame())
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::EncodingThreadProcedure() NOTHING for encoding method");

				toolsObject.SOSleep(10);

				continue;
			}
        
            

			m_pBitRateController->UpdateBitrate();

			llCalculatingTime = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)

			this->m_pColorConverter->ConvertNV12ToI420(m_ucaEncodingFrame);

#elif defined(_DESKTOP_C_SHARP_)

			int iCurWidth = this->m_pColorConverter->GetWidth();
			int iCurHeight = this->m_pColorConverter->GetHeight();

			if (nEncodingFrameSize == iCurWidth * iCurHeight * 2)
			{
				nEncodingFrameSize = this->m_pColorConverter->ConvertYUY2ToI420(m_ucaEncodingFrame, m_ucaConvertedEncodingFrame);
			}
			else if (nEncodingFrameSize == iCurWidth * iCurHeight * 3)
			{
				nEncodingFrameSize = this->m_pColorConverter->ConvertRGB24ToI420(m_ucaEncodingFrame, m_ucaConvertedEncodingFrame);
			}

#elif defined(TARGET_OS_WINDOWS_PHONE)

			if (m_nOrientationType == ORIENTATION_90_MIRRORED)
			{
				this->m_pColorConverter->mirrorRotateAndConvertNV12ToI420(m_ucaEncodingFrame, m_ucaConvertedEncodingFrame);
			}
			else if (m_nOrientationType == ORIENTATION_0_MIRRORED)
			{
				this->m_pColorConverter->mirrorRotateAndConvertNV12ToI420ForBackCam(m_ucaEncodingFrame, m_ucaConvertedEncodingFrame);
			}

#else

			if (m_nOrientationType == ORIENTATION_90_MIRRORED)
			{
				this->m_pColorConverter->mirrorRotateAndConvertNV21ToI420(m_ucaEncodingFrame, m_ucaConvertedEncodingFrame);
			}
			else if (m_nOrientationType == ORIENTATION_0_MIRRORED)
			{
				this->m_pColorConverter->mirrorRotateAndConvertNV21ToI420ForBackCam(m_ucaEncodingFrame, m_ucaConvertedEncodingFrame);
			}

#endif
            
            /*if(m_bIsCheckCall == true)
            {
                memset(m_ucaEncodingFrame, 0, sizeof(m_ucaEncodingFrame));
                
                for(int i=0;i<this->m_pColorConverter->GetHeight();i++)
                {
                    int color = rand()%255;
                    for(int j = 0; j < this->m_pColorConverter->GetWidth(); j ++)
                    {
                        m_ucaEncodingFrame[i * this->m_pColorConverter->GetHeight() + j ] = color;
                    }
                    
                }
            }*/


			CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " Conversion ", llCalculatingTime);

			llCalculatingTime = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG);

#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
            llCalculatingTime = m_Tools.CurrentTimestamp();
            
            if(m_bIsCheckCall)
                nENCODEDFrameSize = m_pVideoEncoder->EncodeVideoFrame(m_ucaDummmyFrame[m_iFrameNumber%3], nEncodingFrameSize, m_ucaEncodedFrame);
            else
                nENCODEDFrameSize = m_pVideoEncoder->EncodeVideoFrame(m_ucaEncodingFrame, nEncodingFrameSize, m_ucaEncodedFrame);
            
            printf("The encoder returned , nENCODEDFrameSize = %d\n", nENCODEDFrameSize);
            

#else
			long timeStampForEncoding = m_Tools.CurrentTimestamp();
			nENCODEDFrameSize = m_pVideoEncoder->EncodeVideoFrame(m_ucaConvertedEncodingFrame, nEncodingFrameSize, m_ucaEncodedFrame);
			int timediff = m_Tools.CurrentTimestamp() - timeStampForEncoding;
			sumOfEncodingTimediff += timeDiff;
			if(nENCODEDFrameSize == 0)
			{
				sumOfZeroLengthEncodingTimediff += timeDiff;
				countZeroLengthFrame++;
			}
			if(m_iFrameNumber % FPS_MAXIMUM == 0)
			{
				CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG, " m_iFrameNumber " + m_Tools.IntegertoStringConvert(m_iFrameNumber%FPS_MAXIMUM) + " Encode time " + m_Tools.IntegertoStringConvert(timeDiff) +
																		  " sumOfEncodingTimediff " + m_Tools.IntegertoStringConvert(sumOfEncodingTimediff ) + " ---  nENCODEDFrameSize  "
																		  + m_Tools.IntegertoStringConvert(nENCODEDFrameSize) + " ---  countZeroLengthFrame  " + m_Tools.IntegertoStringConvert(countZeroLengthFrame)+
																		  " --- ***** afterFrameDropFps  " + m_Tools.IntegertoStringConvert( FPS_MAXIMUM - countZeroLengthFrame));
				sumOfEncodingTimediff = 0;
				countZeroLengthFrame = 0;
			}

#endif
            
            //m_CalculatorEncodeTime.OperationTheatre(llCalculatingTime, m_pVideoCallSession, "Encode");
            //m_TestingEncodeTime.UpdateData(m_Tools.CurrentTimestamp() - llCalculatingTime);
            //CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG || INSTENT_TEST_LOG, "AverageVideoEncoding Time = " + m_Tools.DoubleToString(m_TestingEncodeTime.GetAverage()));
            CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG || INSTENT_TEST_LOG, "VideoEncoding Time = " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp() - llCalculatingTime));
            /*
            if(m_pVideoCallSession->GetCalculationStatus() == true)
            {
                long long currentTime = m_Tools.CurrentTimestamp();
                
                if(currentTime - m_pVideoCallSession->GetCalculationStartTime() <= 1000)
                {
                    m_CalculatorEncodeTime.UpdateData(currentTime - llCalculatingTime);
                }
                else
                {
                    
                    CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG || INSTENT_TEST_LOG, "New EncodeTimeAVg = " + m_Tools.DoubleToString(m_CalculatorEncodeTime.GetAverage()));
                    
                    m_pVideoCallSession->SetCalculationStartMechanism(false);
                }
            }
            */

            
			//CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG || INSTENT_TEST_LOG, " EncodeTime = " + m_Tools.LongLongtoStringConvert(m_Tools.CurrentTimestamp()- llCalculatingTime));

			m_pBitRateController->NotifyEncodedFrame(nENCODEDFrameSize);

			//llCalculatingTime = CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, "" ,true);
            
            
            if(m_FPS_TimeDiff==0)
                m_FPS_TimeDiff = m_Tools.CurrentTimestamp();
            
            if(m_Tools.CurrentTimestamp() -  m_FPS_TimeDiff < 1000 )
            {
                m_FpsCounter++;
            }
            else
            {
                m_FPS_TimeDiff = m_Tools.CurrentTimestamp();
                
                printf("Current Encoding FPS = %d\n", m_FpsCounter);
                if(m_FpsCounter > (FRAME_RATE - FPS_TOLERANCE_FOR_FPS))
                {
                    //kaj korte hobe
                }
                m_FpsCounter = 0;
            }
            
            
			m_pEncodedFramePacketizer->Packetize(m_llFriendID, m_ucaEncodedFrame, nENCODEDFrameSize, m_iFrameNumber, nCaptureTimeDifference);

			//CLogPrinter_WriteLog(CLogPrinter::INFO, OPERATION_TIME_LOG, " Packetize ",true, llCalculatingTime);

			++m_iFrameNumber;
		
			toolsObject.SOSleep(0);
		}
	}

	bEncodingThreadClosed = true;

	CLogPrinter_WriteLog(CLogPrinter::INFO, THREAD_LOG ,"CVideoEncodingThread::EncodingThreadProcedure() stopped EncodingThreadProcedure method.");
}





















//			encodingTimeStamp = toolsObject.CurrentTimestamp();			// called before encoding

/*
encodingTime = toolsObject.CurrentTimestamp() - encodingTimeStamp;

encodeTimeStampFor15 += encodingTime;

countFrameSize = countFrameSize + encodedFrameSize;

if (countFrame >= 15)
{
encodingTimeFahadTest = toolsObject.CurrentTimestamp() - encodingTimeFahadTest;
CLogPrinter_WriteSpecific3(CLogPrinter::DEBUGS, "CVideoEncodingThread::EncodingThreadProcedure() Encoded " + Tools::IntegertoStringConvert(countFrame) + " frames Size: " + Tools::IntegertoStringConvert(countFrameSize * 8) + " encodeTimeStampFor15 : " + Tools::IntegertoStringConvert(encodeTimeStampFor15) + " Full_Lop: " + Tools::IntegertoStringConvert(encodingTimeFahadTest));
encodingTimeFahadTest = toolsObject.CurrentTimestamp();
countFrame = 0;
countFrameSize = 0;
encodeTimeStampFor15 = 0;
}

countFrame++;

dbTotalEncodingTime += encodingTime;
++iEncodedFrameCounter;
nMaxEncodingTime = max(nMaxEncodingTime, encodingTime);
*/