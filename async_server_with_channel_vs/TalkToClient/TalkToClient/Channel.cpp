#include "Channel.h"

bool Channel::HasRoom() const
{
	return true;
}

void Channel::AddClient(TalkToClient::ptr & client)
{
}

void Channel::Start(size_t thread_num)
{
	{
		boost::recursive_mutex::scoped_lock lk(mutex_start_);
		if (started_)
		{
			return;
		}
		started_ = true;
	}

	for (size_t i = 0; i < thread_num; i++)
	{
		threads_.create_thread(MEM_FN(RunService));
	}
}

void Channel::RunService()
{
	service_.run();
}