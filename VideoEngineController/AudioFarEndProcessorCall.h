#ifndef AUDIO_FAR_END_PROCESSOR_CALL_H
#define AUDIO_FAR_END_PROCESSOR_CALL_H


#include "AudioFarEndDataProcessor.h"


class FarEndProcessorCall : public AudioFarEndDataProcessor
{


public:

	FarEndProcessorCall();
	~FarEndProcessorCall() { }

	void ProcessFarEndData();
};



#endif  // !AUDIO_FAR_END_PROCESSOR_CALL_H
