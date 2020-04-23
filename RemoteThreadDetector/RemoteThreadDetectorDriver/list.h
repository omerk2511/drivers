#pragma once

#include <ntddk.h>

#include "fast_mutex.h"

class List
{
public:
	List();
	~List();

	void Insert(LIST_ENTRY* entry);
	void Remove(LIST_ENTRY* entry);

	LIST_ENTRY* get_head();
	FastMutex& get_mutex();

private:
	LIST_ENTRY head_;
	FastMutex mutex_;
	int count_;
};