#pragma once

#include <ntddk.h>

#define PROCMON_DEVICE 0x8000

#define IOCTL_PROCMON_BLOCK_EXECUTABLE CTL_CODE( \
	PROCMON_DEVICE,	0x800, METHOD_NEITHER, FILE_ANY_ACCESS)

namespace config
{
	const UNICODE_STRING kDeviceName = RTL_CONSTANT_STRING(L"\\Device\\ProcMon");
	const UNICODE_STRING kSymbolicLink = RTL_CONSTANT_STRING(L"\\??\\ProcMon");

	const ULONG kDriverTag = 0xdeadbeef;
}