#include "ChannelPool.h"

using namespace BaeServer;

ChannelPool::ptr ChannelPool::instance { nullptr };

Channel::ptr ChannelPool::GetCapableChannel(unsigned short base_port)
{
	Channel::ptr channel { nullptr };

	for (auto & i : channels_)
	{
		if (i.second->HasRoom())
		{
			channel = { i.second };
			break;
		}
	}

	if (nullptr == channel)
	{
		size_t id = id_flag_++;
		unsigned short port = (unsigned short)(base_port + id);
		channel = Channel::NewPtr(id, port);
		channels_._Insert_or_assign(id, channel);

		channel->Start(threads_per_channel);
	}

	return channel;
}

void ChannelPool::AddToChannel(TalkToClient::ptr client, Channel::id_type id)
{
	members_.insert_or_assign(client->id(), id);

	Channel::ptr channel = channels_.find(id)->second;
	channel->AddClient(client);
	client->set_channel_id(channel->id());
}

void ChannelPool::FireDuplicateUser(TalkToClient::id_type id)
{
	auto it = members_.find(id);
	if (bool found = (it != members_.end()))
	{
		auto channel_id = it->second;
		auto channel_it = channels_.find(channel_id);
		if (bool found = (channel_it != channels_.end()))
		{
			auto channel = channel_it->second;
			channel->FireClient(id);
		}

		members_.erase(it);
	}
}
