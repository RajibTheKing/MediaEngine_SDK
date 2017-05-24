#ifndef TRACE_H
#define TRACE_H


namespace MediaSDK
{

	class CTrace
	{
	public:
		static void GenerateTrace(short *sBuffer, int iTraceLength);
		static int DetectTrace(short *sBuffer, int iTraceSearchLength, int iTraceDetectionLength);
	};
} //namespace MediaSDK


#endif //TRACE_H
