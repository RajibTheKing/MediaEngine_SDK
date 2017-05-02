#ifndef AUDIO_FAR_END_PROCESSOR_PUBLISHER_H
#define AUDIO_FAR_END_PROCESSOR_PUBLISHER_H


#include "AudioFarEndDataProcessor.h"


class FarEndProcessorPublisher : public AudioFarEndDataProcessor
{

public:

	FarEndProcessorPublisher();
	~FarEndProcessorPublisher() { }

	void ProcessFarEndData();
};




#endif // !AUDIO_FAR_END_PROCESSOR_PUBLISHER_H
