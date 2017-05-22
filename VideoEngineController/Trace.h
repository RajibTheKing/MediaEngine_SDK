#ifndef TRACE_H
#define TRACE_H

class CTrace
{
public:
	static void GenerateTrace(short *sBuffer, int iTraceLength);
	static bool DetectTrace(short *sBuffer, int iTraceSearchLength, int iTraceDetectionLength);
};

#endif //TRACE_H
