#include "process_create_notification.h"

ProcessCreateNotification::ProcessCreateNotification(ProcessCreateNotifyRoutine notify_routine)
	: notify_routine_(notify_routine)
{
	NTSTATUS status = ::PsSetCreateProcessNotifyRoutineEx(notify_routine, FALSE);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("[-] Failed to set a process notify routine.\n"));
		::ExRaiseStatus(status);
	}
}

ProcessCreateNotification::~ProcessCreateNotification()
{
	::PsSetCreateProcessNotifyRoutineEx(notify_routine_, TRUE);
}
