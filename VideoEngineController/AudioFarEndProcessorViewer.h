#ifndef AUDIO_FAR_END_PROCESSOR_VIEWER_H 
#define AUDIO_FAR_END_PROCESSOR_VIEWER_H


#include "AudioFarEndDataProcessor.h"


class FarEndProcessorViewer : public AudioFarEndDataProcessor
{

public:

	FarEndProcessorViewer();
	~FarEndProcessorViewer() { }

	void ProcessFarEndData();

};


#endif  // !AUDIO_FAR_END_PROCESSOR_VIEWER_H

