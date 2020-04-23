#include "list.h"

List::List()
{
	::InitializeListHead(&head_);
	mutex_.Init();
}

List::~List()
{
	while (!::IsListEmpty(&head_))
	{
		auto entry = ::RemoveTailList(&head_);
		delete entry;
	}
}

void List::Insert(LIST_ENTRY* entry)
{
	::InsertTailList(&head_, entry);
}

void List::Remove(LIST_ENTRY* entry)
{
	::RemoveEntryList(entry);
}
