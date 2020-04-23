#pragma once

#include <ntddk.h>

#include "config.h"
#include "new.h"
#include "list.h"
#include "auto_lock.h"

struct NewProcessEntry
{
	LIST_ENTRY list_entry;
	ULONG process_id;
};

class NewProcessesCache
{
public:
	NewProcessesCache() = default;
	~NewProcessesCache() = default;

	void AddProcess(ULONG process_id);
	bool IsNewlyCreated(ULONG process_id);

private:
	List list_;
};