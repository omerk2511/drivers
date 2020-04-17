#pragma once

#include <ntddk.h>

#include "config.h"
#include "fast_mutex.h"
#include "auto_lock.h"
#include "blocked_image.h"

class BlockedImagesList
{
public:
	BlockedImagesList();
	~BlockedImagesList();

	bool Add(wchar_t name[], unsigned short length);
	void Remove(PUNICODE_STRING name);

	bool IsInList(PUNICODE_STRING name);

private:
	LIST_ENTRY head_;
	FastMutex mutex_;
};