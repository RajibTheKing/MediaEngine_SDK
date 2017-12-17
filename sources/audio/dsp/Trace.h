#ifndef TRACE_H
#define TRACE_H
#include "AudioMacros.h"

namespace MediaSDK
{

	class CTrace
	{
	private:		
		int m_iSentLength;
		short sTraceDetectionBuffer[2 * MAX_AUDIO_FRAME_SAMPLE_SIZE];
	public:
		CTrace();
		void Reset();
		int GenerateTrace(short *sBuffer, int iTraceLength);
		int DetectTrace(short *sBuffer, int iTraceSearchLength, int iTraceDetectionLength, int &iTraceInFrame, bool bUseWaveLength = false);

		int m_iTracePatternLength;
	};
} //namespace MediaSDK


#endif //TRACE_H
