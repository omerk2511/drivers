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

	bool Add(const wchar_t name[], const unsigned short length);
	void Remove(const UNICODE_STRING* name);

	bool IsInList(const UNICODE_STRING* name);

private:
	LIST_ENTRY head_;
	FastMutex mutex_;
};