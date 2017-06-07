#ifndef TRACE_H
#define TRACE_H


namespace MediaSDK
{

	class CTrace
	{
	private:
		int m_iTracePatternLength;
		int m_iSentLength;
	public:
		CTrace();
		int GenerateTrace(short *sBuffer, int iTraceLength);
		int DetectTrace(short *sBuffer, int iTraceSearchLength, int iTraceDetectionLength);
	};
} //namespace MediaSDK


#endif //TRACE_H
