#pragma once

#include <ntddk.h>

#pragma warning(push, 1)
#pragma warning(disable:4200)
struct BlockedImage
{
	LIST_ENTRY entry;
	wchar_t image_name[];
};
#pragma warning(pop)