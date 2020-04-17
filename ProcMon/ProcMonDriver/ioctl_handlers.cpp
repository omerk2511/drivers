#include "ioctl_handlers.h"

void ioctl_handlers::BlockImage(IrpHandler& irp_handler, BlockedImagesList* blocked_images_list)
{
	auto stack = irp_handler.get_stack_location();

	auto buffer = static_cast<wchar_t*>(irp_handler->AssociatedIrp.SystemBuffer);
	auto length = static_cast<unsigned short>(stack->Parameters.DeviceIoControl.InputBufferLength);

	if (length != 0 && buffer != nullptr)
	{
		bool succeeded = blocked_images_list->Add(buffer, length);

		if (!succeeded)
		{
			irp_handler.set_status(STATUS_INSUFFICIENT_RESOURCES);
		}
	}
}
