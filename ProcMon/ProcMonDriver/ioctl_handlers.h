#pragma once

#include <ntddk.h>

#include "config.h"
#include "blocked_images_list.h"
#include "blocked_image.h"
#include "auto_lock.h"
#include "irp_handler.h"

namespace ioctl_handlers
{
	void BlockImage(IrpHandler& irp_handler, BlockedImagesList* blocked_images_list);
	void UnblockImage(IrpHandler& irp_handler, BlockedImagesList* blocked_images_list);
}
