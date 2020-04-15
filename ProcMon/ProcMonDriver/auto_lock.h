#pragma once

template<typename TLock>
class AutoLock
{
public:
	AutoLock(TLock& lock)
		: lock_(lock)
	{
		lock.Lock();
	}

	~AutoLock()
	{
		lock_.Unlock();
	}

private:
	TLock& lock_;
};