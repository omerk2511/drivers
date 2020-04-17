#pragma once

#include <ntddk.h>

#include "config.h"

void* __cdecl operator new(size_t size, POOL_TYPE pool_type = PagedPool, ULONG tag = config::kDriverTag)
{
    auto address = tag ?
        ::ExAllocatePoolWithTag(pool_type, size, tag) :
        ::ExAllocatePool(pool_type, size);

    return address;
}