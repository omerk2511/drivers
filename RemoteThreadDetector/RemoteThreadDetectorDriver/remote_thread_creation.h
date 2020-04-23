#pragma once

#include <ntddk.h>

struct RemoteThreadCreation
{
	ULONG thread_id;
	ULONG process_id;
	ULONG creator_process_id;
};

struct RemoteThreadCreationEntry
{
	LIST_ENTRY list_entry;
	RemoteThreadCreation remote_thread_creation;
};