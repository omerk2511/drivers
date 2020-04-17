#pragma once

#define PROCMON_DEVICE 0x8000

#define IOCTL_PROCMON_BLOCK_IMAGE CTL_CODE( \
	PROCMON_DEVICE,	0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PROCMON_UNBLOCK_IMAGE CTL_CODE( \
	PROCMON_DEVICE,	0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

namespace config
{
	const wchar_t* const kDeviceName = L"\\Device\\ProcMon";
	const wchar_t* const kSymbolicLink = L"\\??\\ProcMon";

	const ULONG kDriverTag = 0xdeadbeef;

	const wchar_t* const kUserModeDeviceName = L"\\\\.\\ProcMon";
}