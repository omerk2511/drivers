#include "new_processes_cache.h"

void NewProcessesCache::AddProcess(ULONG process_id)
{
	AutoLock lock(list_.get_mutex());

	auto entry = new (PagedPool, config::kDriverTag) NewProcessEntry();

	if (!entry)
	{
		::KdPrint(("[-] Failed to log a new process creation detected due to insufficient memory.\n"));
		return;
	}

	entry->process_id = process_id;
	list_.Insert(&entry->list_entry);
}

bool NewProcessesCache::IsNewlyCreated(ULONG process_id)
{
	AutoLock lock(list_.get_mutex());

	auto head = list_.get_head();
	auto current = head->Flink;

	while(current != head)
	{
		auto entry = CONTAINING_RECORD(current, NewProcessEntry, list_entry);
		
		if (process_id == entry->process_id)
		{
			list_.Remove(current);
			return true;
		}

		current = current->Flink;
	}

	return false;
}
