#pragma once

#include "Channel.h"

class ChannelPool
{
	typedef std::vector<Channel::ptr> channel_array;
public:
	ChannelPool() {}
	~ChannelPool() {}

	Channel::ptr ChannelPool::GetCapableChannel();
private:
	channel_array channel_list_;
	boost::mutex cs_;
};

