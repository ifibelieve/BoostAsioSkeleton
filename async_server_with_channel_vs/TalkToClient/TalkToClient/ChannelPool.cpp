#include "ChannelPool.h"

Channel::ptr ChannelPool::GetCapableChannel()
{
	Channel::ptr channel = NULL;

	for (channel_array::const_iterator
		b = channel_list_.begin(),
		e = channel_list_.end(); b != e; b++)
	{
		if ((*b)->HasRoom())
		{
			channel = *b;
			break;
		}
	}

	if (NULL == channel)
	{
		size_t id = channel_list_.size();
		channel = Channel::NewPtr(id);
		channel_list_.push_back(channel);
	}

	return channel;
}