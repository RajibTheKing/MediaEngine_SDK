#ifndef TRACE_H
#define TRACE_H
#include "AudioMacros.h"

namespace MediaSDK
{
	class CAudioDumper;
	class CTrace
	{
	private:		
		int m_iSentLength;
		short m_sTraceDetectionBuffer[2 * MAX_AUDIO_FRAME_SAMPLE_SIZE];
		short m_sSum[2 * MAX_AUDIO_FRAME_SAMPLE_SIZE];
		short m_sMyWL[2 * MAX_AUDIO_FRAME_SAMPLE_SIZE];
		short m_sMyWL_I[2 * MAX_AUDIO_FRAME_SAMPLE_SIZE];
		int m_iTaceWaveCount;

		inline short Diff(short x, short y);

		void CopyFrame(short *sBuffer);
		void CalCulateWaves(int &iWLCount);
		int DetectTrace(int iStartingWave, int iDiffThreshold, int iMatchCountThreshold, int iWLCount, int &iTraceInFrame);

	public:
		CTrace();
		~CTrace();
		void Reset();
		int GenerateTrace(short *sBuffer, int iTraceLength);
		int DetectTrace(short *sBuffer, int &iTraceInFrame);
		int DetectTrace3Times(short *sBuffer, int &iTraceInFrame, bool &b30VerifiedTrace);

		int m_iTracePatternLength;
	};
} //namespace MediaSDK


#endif //TRACE_H
