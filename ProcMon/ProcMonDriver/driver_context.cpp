#include "driver_context.h"

DriverContext::DriverContext(PDRIVER_OBJECT driver_object)
	: device_object_(nullptr)
{
	UNICODE_STRING device_name;
	::RtlInitUnicodeString(&device_name, config::kDeviceName);

	NTSTATUS status = ::IoCreateDevice(
		driver_object,
		0,
		&device_name,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		TRUE,
		&device_object_
	);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("[-] Failed to create a device object.\n"));
		::ExRaiseStatus(status);
	}

	::RtlInitUnicodeString(&symbolic_link_, config::kSymbolicLink);
	status = ::IoCreateSymbolicLink(&symbolic_link_, &device_name);

	if (!NT_SUCCESS(status))
	{
		::IoDeleteDevice(device_object_);
		KdPrint(("[-] Failed to create a symbolic link.\n"));
		::ExRaiseStatus(status);
	}
}

DriverContext::~DriverContext()
{
	::IoDeleteDevice(device_object_);
	::IoDeleteSymbolicLink(&symbolic_link_);
}
