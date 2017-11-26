#ifndef TRACE_H
#define TRACE_H


namespace MediaSDK
{

	class CTrace
	{
	private:		
		int m_iSentLength;
	public:
		CTrace();
		void Reset();
		int GenerateTrace(short *sBuffer, int iTraceLength);
		int DetectTrace(short *sBuffer, int iTraceSearchLength, int iTraceDetectionLength);

		int m_iTracePatternLength;
	};
} //namespace MediaSDK


#endif //TRACE_H
