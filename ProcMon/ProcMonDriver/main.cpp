#include "config.h"
#include "definitions.h"

#include <ntddk.h>

void ProcMonUnload(PDRIVER_OBJECT);

NTSTATUS ProcMonCreateClose(PDEVICE_OBJECT, PIRP);
NTSTATUS ProcMonDeviceControl(PDEVICE_OBJECT, PIRP);

void ProcMonProcessNotify(PEPROCESS, HANDLE, PPS_CREATE_NOTIFY_INFO);

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

	status = ::PsSetCreateProcessNotifyRoutineEx(ProcMonProcessNotify, FALSE);

	if (!NT_SUCCESS(status))
	{
		::IoDeleteDevice(device_object);
		::IoDeleteSymbolicLink(const_cast<UNICODE_STRING*>(&SYMBOLIC_LINK));

		KdPrint(("[-] Failed to set a process notify routine.\n"));
		return status;
	}

	KdPrint(("[+] Loaded ProcMon successfully.\n"));
	return STATUS_SUCCESS;
}

void ProcMonUnload(PDRIVER_OBJECT driver_object)
{
	::IoDeleteSymbolicLink(const_cast<UNICODE_STRING*>(&SYMBOLIC_LINK));
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

void ProcMonProcessNotify(PEPROCESS process, HANDLE process_id, PPS_CREATE_NOTIFY_INFO create_info)
{
	if (create_info)
	{
		
	}
}