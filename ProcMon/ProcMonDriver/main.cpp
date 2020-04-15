#include <ntddk.h>
#include "config.h"

void ProcMonUnload(PDRIVER_OBJECT);

NTSTATUS ProcMonCreateClose(PDEVICE_OBJECT, PIRP);
NTSTATUS ProcMonDeviceControl(PDEVICE_OBJECT, PIRP);

const UNICODE_STRING DEVICE_NAME = RTL_CONSTANT_STRING(L"\\Device\\ProcMon");
const UNICODE_STRING SYMBOLIC_LINK = RTL_CONSTANT_STRING(L"\\??\\ProcMon");

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING)
{
	driver_object->DriverUnload = ProcMonUnload;

	driver_object->MajorFunction[IRP_MJ_CREATE] = ProcMonCreateClose;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = ProcMonCreateClose;
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ProcMonDeviceControl;

	PDEVICE_OBJECT device_object;

	auto status = ::IoCreateDevice(
		driver_object,
		0,
		const_cast<UNICODE_STRING*>(&DEVICE_NAME),
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&device_object
	);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("[-] Failed to create a device object.\n"));
		return status;
	}

	status = ::IoCreateSymbolicLink(
		const_cast<UNICODE_STRING*>(&SYMBOLIC_LINK),
		const_cast<UNICODE_STRING*>(&DEVICE_NAME)
	);

	if (!NT_SUCCESS(status))
	{
		::IoDeleteDevice(device_object);
		KdPrint(("[-] Failed to create a symbolic link.\n"));
		return status;
	}

	return STATUS_SUCCESS;
}

void ProcMonUnload(PDRIVER_OBJECT driver_object)
{
	::IoDeleteSymbolicLink(const_cast<UNICODE_STRING*>(&SYMBOLIC_LINK));
	::IoDeleteDevice(driver_object->DeviceObject);
}