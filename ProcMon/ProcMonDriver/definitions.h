#pragma once

#include <ntddk.h>

struct Globals
{
	LIST_ENTRY blocked_images_list_head;
	FAST_MUTEX blocked_images_list_mutex;
};

struct BlockedImage
{
	LIST_ENTRY entry;
	unsigned short image_name_length;
	wchar_t image_name[];
};