
#ifndef IPV_UTILS_H
#define IPV_UTILS_H

#include <string>
#include <vector>
#include <thread>

#include "AudioVideoEngineDefinitions.h"

#ifndef WIN32
#include <errno.h>
#endif

#ifndef _MSC_VER
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

#define SHARED_PTR_DELETE(sharedPtr) {/*if (sharedPtr.get()) */sharedPtr.reset();}

#endif
