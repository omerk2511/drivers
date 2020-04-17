#pragma once

#include <string>
#include <windows.h>

class Device
{
public:
	Device(const std::wstring& device_name);
	~Device();

	bool ioctl(uint32_t control_code, void* input_buf, size_t input_size,
		void* output_buf, size_t output_size, DWORD* bytes_returned = nullptr);

private:
	HANDLE handle_;
};