#pragma once

#include "Channel.h"

class ChannelPool
{
	typedef std::map<Channel::id_type, Channel::ptr> ChannelMap;
	ChannelPool() : id_flag_(1) {}
	
	static ChannelPool * instance;
public:
	static ChannelPool * GetInstance() 
	{
		if (nullptr == instance)
		{
			instance = new ChannelPool();
		}

		return instance;
	}

	Channel::ptr GetCapableChannel(unsigned short base_port, size_t thread_num);
	void AddToChannel(TalkToClient::ptr client, Channel::id_type id);
private:
	size_t id_flag_;
	ChannelMap channel_map_;
	boost::mutex cs_;
};

