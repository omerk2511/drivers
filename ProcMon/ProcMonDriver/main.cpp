#include <ntddk.h>

#include "config.h"
#include "new.h"
#include "delete.h"
#include "blocked_images_list.h"
#include "blocked_image.h"
#include "irp_handler.h"
#include "ioctl_handlers.h"

void ProcMonUnload(PDRIVER_OBJECT);

NTSTATUS ProcMonCreateClose(PDEVICE_OBJECT, PIRP);
NTSTATUS ProcMonDeviceControl(PDEVICE_OBJECT, PIRP);

void ProcMonProcessNotify(PEPROCESS, HANDLE, PPS_CREATE_NOTIFY_INFO);

BlockedImagesList* g_blocked_images_list;

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING)
{
	driver_object->DriverUnload = ProcMonUnload;

	driver_object->MajorFunction[IRP_MJ_CREATE] = ProcMonCreateClose;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = ProcMonCreateClose;
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ProcMonDeviceControl;

	PDEVICE_OBJECT device_object;

	UNICODE_STRING device_name;
	::RtlInitUnicodeString(&device_name, config::kDeviceName);

	auto status = ::IoCreateDevice(
		driver_object,
		0,
		&device_name,
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

	UNICODE_STRING symbolic_link;
	::RtlInitUnicodeString(&symbolic_link, config::kSymbolicLink);

	status = ::IoCreateSymbolicLink(&symbolic_link,	&device_name);

	if (!NT_SUCCESS(status))
	{
		::IoDeleteDevice(device_object);
		KdPrint(("[-] Failed to create a symbolic link.\n"));
		return status;
	}

	status = ::PsSetCreateProcessNotifyRoutineEx(ProcMonProcessNotify, FALSE);

	if (!NT_SUCCESS(status))
	{
		::IoDeleteDevice(device_object);
		::IoDeleteSymbolicLink(&symbolic_link);

		KdPrint(("[-] Failed to set a process notify routine.\n"));
		return status;
	}

	g_blocked_images_list = new (PagedPool, config::kDriverTag) BlockedImagesList();

	if (!g_blocked_images_list)
	{
		::IoDeleteDevice(device_object);
		::IoDeleteSymbolicLink(&symbolic_link);
		::PsSetCreateProcessNotifyRoutineEx(ProcMonProcessNotify, TRUE);

		KdPrint(("[-] Failed to create a blocked images list.\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	KdPrint(("[+] Loaded ProcMon successfully.\n"));
	return STATUS_SUCCESS;
}

void ProcMonUnload(PDRIVER_OBJECT driver_object)
{
	UNICODE_STRING symbolic_link;
	::RtlInitUnicodeString(&symbolic_link, config::kSymbolicLink);

	::IoDeleteSymbolicLink(&symbolic_link);
	::IoDeleteDevice(driver_object->DeviceObject);

	::PsSetCreateProcessNotifyRoutineEx(ProcMonProcessNotify, TRUE);

	delete g_blocked_images_list;

	KdPrint(("[+] Unloaded ProcMon successfully.\n"));
}

NTSTATUS ProcMonCreateClose(PDEVICE_OBJECT, PIRP irp)
{
	IrpHandler irp_handler(irp);
	return irp_handler.get_status();
}

NTSTATUS ProcMonDeviceControl(PDEVICE_OBJECT, PIRP irp)
{
	IrpHandler irp_handler(irp);

	auto stack = irp_handler.get_stack_location();
	auto ioctl = stack->Parameters.DeviceIoControl.IoControlCode;

	switch (ioctl)
	{
	case IOCTL_PROCMON_BLOCK_IMAGE:
	{
		ioctl_handlers::BlockImage(irp_handler, g_blocked_images_list);
		break;
	}

	default:
	{
		irp_handler.set_status(STATUS_INVALID_DEVICE_REQUEST);
		break;
	}
	}

	return irp_handler.get_status();
}

void ProcMonProcessNotify(PEPROCESS, HANDLE process_id, PPS_CREATE_NOTIFY_INFO create_info)
{
	UNREFERENCED_PARAMETER(process_id); // TODO: check if necessary

	if (create_info && create_info->FileOpenNameAvailable)
	{
		if (g_blocked_images_list->IsInList(const_cast<PUNICODE_STRING>(create_info->ImageFileName)))
		{
			KdPrint(("[*] Blocking process %d with image %wZ from being created.\n", process_id, create_info->ImageFileName));
			create_info->CreationStatus = STATUS_ACCESS_DENIED;
		}
	}
}