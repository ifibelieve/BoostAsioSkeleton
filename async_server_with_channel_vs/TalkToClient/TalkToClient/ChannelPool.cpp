#include "ChannelPool.h"

Channel::ptr ChannelPool::GetCapableChannel()
{
	Channel::ptr channel = NULL;

	for (ChannelMap::const_iterator
		b = channel_map_.begin(),
		e = channel_map_.end(); b != e; b++)
	{
		if ((b->second)->HasRoom())
		{
			channel = b->second;
			break;
		}
	}

	if (NULL == channel)
	{
		size_t id = id_flag_++;
		channel = Channel::NewPtr(id);
		channel_map_._Insert_or_assign(id, channel);
	}

	return channel;
}