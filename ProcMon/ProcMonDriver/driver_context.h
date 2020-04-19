#pragma once

#include <ntddk.h>

#include "config.h"

class DriverContext
{
public:
	DriverContext(PDRIVER_OBJECT driver_object);
	~DriverContext();

private:
	PDEVICE_OBJECT device_object_;
	UNICODE_STRING symbolic_link_;
};
