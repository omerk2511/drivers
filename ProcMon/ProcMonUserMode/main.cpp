#include <iostream>
#include <string>
#include <windows.h>

#include <config.h>
#include "device.h"

int main(int argc, char* argv[])
{
	if (argc != 3 || (std::string(argv[1]) != "-b" && std::string(argv[1]) != "-u"))
	{
		std::cout << "Usage: " << argv[0] << " [-b | -u] image" << std::endl;
		return 1;
	}

	try
	{
		Device device(config::kUserModeDeviceName);

		std::string image_name(argv[2]);
		std::wstring u_image_name(image_name.begin(), image_name.end());

		if (std::string(argv[1]) == "-b")
		{
			bool succeeded = device.ioctl(
				IOCTL_PROCMON_BLOCK_IMAGE,
				static_cast<LPVOID>(const_cast<wchar_t*>(u_image_name.c_str())),
				static_cast<DWORD>(u_image_name.size() * sizeof(wchar_t)),
				nullptr,
				0
			);

			if (!succeeded)
			{
				std::cout << "[-] Unable to add the image to the blocked images list." << std::endl;
				return 1;
			}

			std::cout << "[+] Added image " << argv[2] << " to the blocked images list." << std::endl;
		}
		else
		{
			bool succeeded = device.ioctl(
				IOCTL_PROCMON_UNBLOCK_IMAGE,
				static_cast<LPVOID>(const_cast<wchar_t*>(u_image_name.c_str())),
				static_cast<DWORD>(u_image_name.size() * sizeof(wchar_t)),
				nullptr,
				0
			);

			if (!succeeded)
			{
				std::cout << "[-] Unable to remove the image from the blocked images list." << std::endl;
				return 1;
			}

			std::cout << "[+] Removed image " << argv[2] << " from the blocked images list." << std::endl;
		}
	}
	catch(std::exception& e)
	{
		std::cout << "[-] " << e.what() << std::endl;
	}

	return 0;
}