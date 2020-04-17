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

	InitializeListHead(&globals.blocked_images_list_head);
	globals.blocked_images_list_mutex.Init();

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

	while (!IsListEmpty(&globals.blocked_images_list_head))
	{
		auto entry = RemoveTailList(&globals.blocked_images_list_head);
		BlockedImage* blocked_image = CONTAINING_RECORD(entry, BlockedImage, entry);

		ExFreePool(blocked_image);
	}

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
		AutoLock lock(globals.blocked_images_list_mutex);

		auto length = static_cast<unsigned short>(stack->Parameters.DeviceIoControl.InputBufferLength);
		auto buffer = irp->AssociatedIrp.SystemBuffer;

		if (length != 0 && buffer != nullptr)
		{
			auto blocked_image = static_cast<BlockedImage*>(
				::ExAllocatePoolWithTag(
					PagedPool,
					sizeof(BlockedImage) + length + sizeof(wchar_t),
					config::kDriverTag
				)
			);

			if (!blocked_image)
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			blocked_image->image_name_length = length;

			::memcpy(blocked_image->image_name, buffer, length);
			blocked_image->image_name[length / 2] = 0;

			::InsertTailList(&globals.blocked_images_list_head, &blocked_image->entry);

			KdPrint(("[+] Added the %ws image to the blocked images list.\n", blocked_image->image_name));
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
	UNREFERENCED_PARAMETER(process_id);

	if (create_info && create_info->FileOpenNameAvailable)
	{
		AutoLock lock(globals.blocked_images_list_mutex);

		LIST_ENTRY* head = &globals.blocked_images_list_head;
		LIST_ENTRY* current = head->Flink;

		bool should_be_blocked = false;

		while (current != head)
		{
			BlockedImage* blocked_image = CONTAINING_RECORD(current, BlockedImage, entry);

			UNICODE_STRING blocked_image_name;
			::RtlInitUnicodeString(&blocked_image_name, blocked_image->image_name);

			long equal = ::RtlCompareUnicodeString(
				&blocked_image_name,
				create_info->ImageFileName,
				false
			);

			if (equal == 0)
			{
				should_be_blocked = true;
				break;
			}

			current = current->Flink;
		}

		if (should_be_blocked)
		{
			KdPrint(("[*] Blocking process %d with image %wZ from being created.\n", process_id, create_info->ImageFileName));
			create_info->CreationStatus = STATUS_ACCESS_DENIED;
		}
	}
}