#include "fast_mutex.h"

void FastMutex::Init()
{
	ExInitializeFastMutex(&mutex_);
}

void FastMutex::Lock()
{
	ExAcquireFastMutex(&mutex_);
}

void FastMutex::Unlock()
{
	ExReleaseFastMutex(&mutex_);
}
