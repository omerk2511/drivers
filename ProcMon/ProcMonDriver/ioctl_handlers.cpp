#include "ioctl_handlers.h"

void ioctl_handlers::BlockImage(IrpHandler& irp_handler, Globals& globals)
{
	AutoLock lock(globals.blocked_images_list_mutex);

	auto stack = irp_handler.get_stack_location();

	auto length = static_cast<unsigned short>(stack->Parameters.DeviceIoControl.InputBufferLength);
	auto buffer = irp_handler->AssociatedIrp.SystemBuffer;

	if (length != 0 && buffer != nullptr)
	{
		auto blocked_image = static_cast<BlockedImage*>(
			::ExAllocatePoolWithTag(
				PagedPool,
				sizeof(BlockedImage) + length + sizeof(wchar_t) * 5,
				config::kDriverTag
			)
		);

		if (!blocked_image)
		{
			irp_handler.set_status(STATUS_INSUFFICIENT_RESOURCES);
			return;
		}

		blocked_image->image_name_length = length;

		::memcpy(blocked_image->image_name, L"\\??\\", 8);
		::memcpy(blocked_image->image_name + 4, buffer, length);
		blocked_image->image_name[length / 2 + 4] = 0;

		::InsertTailList(&globals.blocked_images_list_head, &blocked_image->entry);

		KdPrint(("[+] Added the %ws image to the blocked images list.\n", blocked_image->image_name));
	}
}
