#include "ChannelPool.h"

ChannelPool * ChannelPool::instance = nullptr;

Channel::ptr ChannelPool::GetCapableChannel(unsigned short base_port, size_t thread_num)
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
		size_t port = base_port + id;
		channel = Channel::NewPtr(id, port);
		channel_map_._Insert_or_assign(id, channel);

		channel->Start(thread_num);
	}

	return channel;
}

void ChannelPool::AddToChannel(TalkToClient::ptr client, Channel::id_type id)
{
	Channel::ptr channel = channel_map_.find(id)->second;
	channel->AddClient(client);
	client->set_channel_id(channel->id());
}
