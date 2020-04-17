#include "device.h"

Device::Device(const std::wstring& device_name)
{
	handle_ = ::CreateFile(
		device_name.c_str(),
		GENERIC_ALL,
		0,
		nullptr,
		OPEN_EXISTING,
		0,
		nullptr
	);

	if (handle_ == INVALID_HANDLE_VALUE)
	{
		throw std::exception("Could not open device.");
	}
}

Device::~Device()
{
	if (handle_ != INVALID_HANDLE_VALUE)
		::CloseHandle(handle_);
}

bool Device::ioctl(uint32_t ioctl_code, void* input_buf, size_t input_size, void* output_buf, size_t output_size, DWORD* bytes_returned)
{
	bool success = ::DeviceIoControl(
		handle_,
		ioctl_code,
		input_buf,
		input_size,
		output_buf,
		output_size,
		bytes_returned,
		nullptr
	);

	return success;
}
