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

bool BlockedImagesList::Remove(const wchar_t name[], const unsigned short length)
{
	AutoLock lock(mutex_);

	wchar_t *actual_name = static_cast<wchar_t*>(
		::ExAllocatePoolWithTag(
			PagedPool,
			sizeof(BlockedImage) + length + sizeof(wchar_t) * 5,
			config::kDriverTag
		)
	);

	if (!actual_name)
	{
		KdPrint(("[-] Failed to remove an image from the blocked images list.\n"));
		return false;
	}

	::memcpy(actual_name, L"\\??\\", 8);
	::memcpy(actual_name + 4, name, length);
	actual_name[length / 2 + 4] = 0;

	UNICODE_STRING u_name;
	::RtlInitUnicodeString(&u_name, actual_name);

	LIST_ENTRY* current = head_.Flink;

	while (current != &head_)
	{
		BlockedImage* blocked_image = CONTAINING_RECORD(current, BlockedImage, entry);

		UNICODE_STRING blocked_image_name;
		::RtlInitUnicodeString(&blocked_image_name, blocked_image->image_name);

		long equal = ::RtlCompareUnicodeString(
			&blocked_image_name,
			&u_name,
			false
		);

		if (equal == 0)
		{
			::RemoveEntryList(current);
			delete blocked_image;

			KdPrint(("[+] Removed the %ws image from the blocked images list.\n", actual_name));

			return true;
		}

		current = current->Flink;
	}

	KdPrint(("[-] Failed to remove the %ws image from the blocked images list.\n", actual_name));

	return false;
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
