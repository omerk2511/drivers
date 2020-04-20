#pragma once

namespace config
{
	const wchar_t* const kDeviceName = L"\\Device\\RemoteThreadDetector";
	const wchar_t* const kSymbolicLink = L"\\??\\RemoteThreadDetector";

	const ULONG kDriverTag = 0xdeadbeef;

	const wchar_t* const kUserModeDeviceName = L"\\\\.\\RemoteThreadDetector";
}