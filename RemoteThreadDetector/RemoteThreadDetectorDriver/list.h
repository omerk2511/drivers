#pragma once

#include <ntddk.h>

#include "fast_mutex.h"
#include "auto_lock.h"

class List
{
public:
	List();
	~List();

	void Insert(LIST_ENTRY* entry);
	void Remove(LIST_ENTRY* entry);

	const LIST_ENTRY* operator->() const
	{
		return &head_;
	}

private:
	LIST_ENTRY head_;
	FastMutex mutex_;
};