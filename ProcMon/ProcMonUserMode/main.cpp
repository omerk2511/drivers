#include <iostream>
#include <string>
#include <windows.h>

#include <config.h>

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " [image]" << std::endl;
		return 1;
	}

	HANDLE device_handle = ::CreateFile(
		config::kUserModeDeviceName,
		GENERIC_ALL,
		0,
		nullptr,
		OPEN_EXISTING,
		0,
		nullptr
	);

	if (device_handle == INVALID_HANDLE_VALUE)
	{
		std::cout << "[-] Unable to open device." << std::endl;
		return 1;
	}

	std::string image_name(argv[1]);
	std::wstring u_image_name(image_name.begin(), image_name.end());

	bool succeeded = ::DeviceIoControl(
		device_handle,
		IOCTL_PROCMON_BLOCK_IMAGE,
		static_cast<LPVOID>(const_cast<wchar_t*>(u_image_name.c_str())),
		static_cast<DWORD>(u_image_name.size() * sizeof(wchar_t)),
		nullptr,
		0,
		nullptr,
		nullptr
	);

	if (!succeeded)
	{
		std::cout << "[-] Unable to add the image to the blocked images list." << std::endl;
		return 1;
	}

	std::cout << "[+] Added image " << argv[1] << " to the blocked images list." << std::endl;

	::CloseHandle(device_handle);
}