#include <ntddk.h>

#include "config.h"
#include "new.h"
#include "delete.h"
#include "driver_context.h"
#include "process_create_notification.h"
#include "blocked_images_list.h"
#include "blocked_image.h"
#include "irp_handler.h"
#include "ioctl_handlers.h"

void ProcMonUnload(PDRIVER_OBJECT);

NTSTATUS ProcMonCreateClose(PDEVICE_OBJECT, PIRP);
NTSTATUS ProcMonDeviceControl(PDEVICE_OBJECT, PIRP);

void ProcMonProcessNotify(PEPROCESS, HANDLE, PPS_CREATE_NOTIFY_INFO);

DriverContext* g_driver_context;
ProcessCreateNotification* g_process_create_notification;
BlockedImagesList* g_blocked_images_list;

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING)
{
	__try
	{
		driver_object->DriverUnload = ProcMonUnload;

		driver_object->MajorFunction[IRP_MJ_CREATE] = ProcMonCreateClose;
		driver_object->MajorFunction[IRP_MJ_CLOSE] = ProcMonCreateClose;
		driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ProcMonDeviceControl;

		g_driver_context = new (PagedPool, config::kDriverTag) DriverContext(driver_object);
		g_blocked_images_list = new (PagedPool, config::kDriverTag) BlockedImagesList();
		g_process_create_notification = new (PagedPool, config::kDriverTag) ProcessCreateNotification(ProcMonProcessNotify);

		KdPrint(("[+] Loaded ProcMon successfully.\n"));
		return STATUS_SUCCESS;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("[-] Caught exception %d.\n", ::GetExceptionCode()));
		return STATUS_FAILED_DRIVER_ENTRY;
	}
}

void ProcMonUnload(PDRIVER_OBJECT)
{
	delete g_driver_context;
	delete g_process_create_notification;
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

	case IOCTL_PROCMON_UNBLOCK_IMAGE:
	{
		ioctl_handlers::UnblockImage(irp_handler, g_blocked_images_list);
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
	UNREFERENCED_PARAMETER(process_id);

	if (create_info && create_info->FileOpenNameAvailable)
	{
		if (g_blocked_images_list->IsInList(create_info->ImageFileName))
		{
			KdPrint(("[*] Blocking process %d with image %wZ from being created.\n", process_id, create_info->ImageFileName));
			create_info->CreationStatus = STATUS_ACCESS_DENIED;
		}
	}
}