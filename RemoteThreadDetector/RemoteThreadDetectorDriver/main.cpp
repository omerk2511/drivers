#include <ntddk.h>

#include "config.h"

void DriverUnload(PDRIVER_OBJECT);

NTSTATUS CreateCloseDispatch(PDEVICE_OBJECT, PIRP);
NTSTATUS ReadDispatch(PDEVICE_OBJECT, PIRP);

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING)
{
	driver_object->DriverUnload = DriverUnload;

	driver_object->MajorFunction[IRP_MJ_CREATE] = CreateCloseDispatch;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = CreateCloseDispatch;
	driver_object->MajorFunction[IRP_MJ_READ] = ReadDispatch;

	UNICODE_STRING device_name;
	::RtlInitUnicodeString(&device_name, config::kDeviceName);

	PDEVICE_OBJECT device_object;

	NTSTATUS status = ::IoCreateDevice(
		driver_object,
		0,
		&device_name,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		true,
		&device_object
	);

	if (!NT_SUCCESS(status))
	{
		::KdPrint(("[-] Failed to create a device object.\n"));
		return status;
	}

	UNICODE_STRING symbolic_link;
	::RtlInitUnicodeString(&symbolic_link, config::kSymbolicLink);

	status = ::IoCreateSymbolicLink(
		&symbolic_link,
		&device_name
	);

	if (!NT_SUCCESS(status))
	{
		::IoDeleteDevice(device_object);
		::KdPrint(("[-] Failed to create a symbolic link.\n"));
		return status;
	}

	::KdPrint(("[+] Loaded RemoteThreadDetector successfully.\n"));

	return STATUS_SUCCESS;
}

void DriverUnload(PDRIVER_OBJECT driver_object)
{
	UNICODE_STRING symbolic_link;
	::RtlInitUnicodeString(&symbolic_link, config::kSymbolicLink);

	::IoDeleteDevice(driver_object->DeviceObject);
	::IoDeleteSymbolicLink(&symbolic_link);

	::KdPrint(("[+] Unloaded RemoteThreadDetector successfully.\n"));
}

NTSTATUS CreateCloseDispatch(PDEVICE_OBJECT, PIRP irp)
{
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	::IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS ReadDispatch(PDEVICE_OBJECT, PIRP irp)
{
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	::IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}