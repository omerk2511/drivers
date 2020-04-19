#pragma once

#include <ntddk.h>

typedef void (*ProcessCreateNotifyRoutine)(PEPROCESS, HANDLE, PPS_CREATE_NOTIFY_INFO);

class ProcessCreateNotification
{
public:
	ProcessCreateNotification(ProcessCreateNotifyRoutine notify_routine);
	~ProcessCreateNotification();

private:
	ProcessCreateNotifyRoutine notify_routine_;
};
