#ifndef AUDIO_FAR_END_PROCESSOR_CHANNEL_H
#define AUDIO_FAR_END_PROCESSOR_CHANNEL_H

#include "AudioFarEndDataProcessor.h"


class FarEndProcessorChannel : public AudioFarEndDataProcessor
{

public:

	FarEndProcessorChannel();
	~FarEndProcessorChannel() { }

	void ProcessFarEndData();

};



#endif  // !AUDIO_FAR_END_PROCESSOR_CHANNEL_H
