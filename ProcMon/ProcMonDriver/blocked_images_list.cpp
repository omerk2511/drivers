#include "blocked_images_list.h"

BlockedImagesList::BlockedImagesList()
{
	::InitializeListHead(&head_);
	mutex_.Init();
}

BlockedImagesList::~BlockedImagesList()
{
	while (!IsListEmpty(&head_))
	{
		auto entry = ::RemoveTailList(&head_);
		BlockedImage* blocked_image = CONTAINING_RECORD(entry, BlockedImage, entry);

		delete blocked_image;
	}
}

bool BlockedImagesList::Add(const wchar_t name[], const unsigned short length)
{
	AutoLock lock(mutex_);

	auto blocked_image = static_cast<BlockedImage*>(
		::ExAllocatePoolWithTag(
			PagedPool,
			sizeof(BlockedImage) + length + sizeof(wchar_t) * 5,
			config::kDriverTag
		)
	);

	if (!blocked_image)
	{
		KdPrint(("[-] Failed to add an image to the blocked images list.\n"));
		return false;
	}

	::memcpy(blocked_image->image_name, L"\\??\\", 8);
	::memcpy(blocked_image->image_name + 4, name, length);
	blocked_image->image_name[length / 2 + 4] = 0;

	::InsertTailList(&head_, &blocked_image->entry);

	KdPrint(("[+] Added the %ws image to the blocked images list.\n", blocked_image->image_name));

	return true;
}

void BlockedImagesList::Remove(const UNICODE_STRING* name)
{
	AutoLock lock(mutex_);

	LIST_ENTRY* current = head_.Flink;

	while (current != &head_)
	{
		BlockedImage* blocked_image = CONTAINING_RECORD(current, BlockedImage, entry);

		UNICODE_STRING blocked_image_name;
		::RtlInitUnicodeString(&blocked_image_name, blocked_image->image_name);

		long equal = ::RtlCompareUnicodeString(
			&blocked_image_name,
			name,
			false
		);

		if (equal == 0)
		{
			::RemoveEntryList(current);
			delete blocked_image;

			KdPrint(("[+] Removed the %Z image to the blocked images list.\n", name));

			break;
		}

		current = current->Flink;
	}
}

bool BlockedImagesList::IsInList(const UNICODE_STRING* name)
{
	AutoLock lock(mutex_);

	LIST_ENTRY* current = head_.Flink;

	while (current != &head_)
	{
		BlockedImage* blocked_image = CONTAINING_RECORD(current, BlockedImage, entry);

		UNICODE_STRING blocked_image_name;
		::RtlInitUnicodeString(&blocked_image_name, blocked_image->image_name);

		long equal = ::RtlCompareUnicodeString(
			&blocked_image_name,
			name,
			false
		);

		if (equal == 0)
		{
			return true;
		}

		current = current->Flink;
	}

	return false;
}
