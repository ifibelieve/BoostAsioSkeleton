#pragma once

#include "Channel.h"

class ChannelPool
{
	typedef std::map<size_t, Channel::ptr> ChannelMap;
public:
	ChannelPool() : id_flag_(0) {}
	~ChannelPool() {}

	Channel::ptr ChannelPool::GetCapableChannel();
private:
	size_t id_flag_;
	ChannelMap channel_map_;
	boost::mutex cs_;
};

