#pragma once

#include <ntddk.h>

#include "config.h"
#include "definitions.h"
#include "auto_lock.h"
#include "irp_handler.h"

namespace ioctl_handlers
{
	void BlockImage(IrpHandler& irp_handler, Globals& globals);
}
