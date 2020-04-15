#include <ntddk.h>

#include "config.h"
#include "definitions.h"
#include "auto_lock.h"

void ProcMonUnload(PDRIVER_OBJECT);

NTSTATUS ProcMonCreateClose(PDEVICE_OBJECT, PIRP);
NTSTATUS ProcMonDeviceControl(PDEVICE_OBJECT, PIRP);

void ProcMonProcessNotify(PEPROCESS, HANDLE, PPS_CREATE_NOTIFY_INFO);

Globals globals;

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
		const_cast<UNICODE_STRING*>(&config::kDeviceName),
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
		const_cast<UNICODE_STRING*>(&config::kSymbolicLink),
		const_cast<UNICODE_STRING*>(&config::kDeviceName)
	);

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
		::IoDeleteSymbolicLink(const_cast<UNICODE_STRING*>(&config::kSymbolicLink));

		KdPrint(("[-] Failed to set a process notify routine.\n"));
		return status;
	}

	InitializeListHead(&globals.blocked_images_list_head);
	globals.blocked_images_list_mutex.Init();

	KdPrint(("[+] Loaded ProcMon successfully.\n"));
	return STATUS_SUCCESS;
}

void ProcMonUnload(PDRIVER_OBJECT driver_object)
{
	::IoDeleteSymbolicLink(const_cast<UNICODE_STRING*>(&config::kSymbolicLink));
	::IoDeleteDevice(driver_object->DeviceObject);
	::PsSetCreateProcessNotifyRoutineEx(ProcMonProcessNotify, TRUE);

	KdPrint(("[+] Unloaded ProcMon successfully.\n"));
}

NTSTATUS ProcMonCreateClose(PDEVICE_OBJECT, PIRP irp)
{
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	::IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS ProcMonDeviceControl(PDEVICE_OBJECT, PIRP irp)
{
	auto stack = ::IoGetCurrentIrpStackLocation(irp);
	auto ioctl = stack->Parameters.DeviceIoControl.IoControlCode;

	irp->IoStatus.Information = 0;
	auto status = STATUS_SUCCESS;

	switch (ioctl)
	{
	case IOCTL_PROCMON_BLOCK_EXECUTABLE:
	{
		auto length = static_cast<unsigned short>(stack->Parameters.DeviceIoControl.InputBufferLength);
		auto buffer = stack->Parameters.DeviceIoControl.Type3InputBuffer;

		if (length != 0 && buffer != nullptr)
		{
			// validate buffer

			auto blocked_image = static_cast<BlockedImage*>(
				::ExAllocatePoolWithTag(
					PagedPool,
					sizeof(BlockedImage) + length,
					config::kDriverTag
				)
			);

			if (!blocked_image)
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			blocked_image->image_name_length = length;
			memcpy(blocked_image->image_name, buffer, length);

			::InsertTailList(&globals.blocked_images_list_head, &blocked_image->entry);

			KdPrint(("[+] Add the %S image to the blocked images list.\n", blocked_image->image_name));
		}

		break;
	}

	default:
	{
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}
	}

	irp->IoStatus.Status = status;
	::IoCompleteRequest(irp, IO_NO_INCREMENT);

	return status;
}

void ProcMonProcessNotify(PEPROCESS, HANDLE process_id, PPS_CREATE_NOTIFY_INFO create_info)
{
	if (create_info)
	{
		LIST_ENTRY* head = &globals.blocked_images_list_head;
		LIST_ENTRY* current = head->Flink;

		bool should_be_blocked = false;

		while (current != head)
		{
			BlockedImage* blocked_image = CONTAINING_RECORD(current, BlockedImage, entry);

			UNICODE_STRING blocked_image_name;
			::RtlInitUnicodeString(&blocked_image_name, blocked_image->image_name);

			bool equal = ::RtlCompareUnicodeString(
				create_info->ImageFileName,
				&blocked_image_name,
				false
			);

			if (equal)
			{
				should_be_blocked = true;
				break;
			}

			current = current->Flink;
		}

		if (should_be_blocked)
		{
			KdPrint(("[*] Blocking process %d with image %Z from being created.\n", process_id, create_info->ImageFileName));
			create_info->CreationStatus = STATUS_ACCESS_DENIED;
		}
	}
}