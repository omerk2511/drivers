#pragma once

#include <ntddk.h>

struct RemoteThreadCreation
{
	HANDLE thread_id;
	HANDLE process_id;
	HANDLE creator_process_id;
};

struct RemoteThreadCreationEntry
{
	LIST_ENTRY list_entry;
	RemoteThreadCreation remote_thread_creation;
};