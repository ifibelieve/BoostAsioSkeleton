#pragma once

#include <map>
#include <mutex>
#include <memory>

#include "Channel.h"
#include "TalkToClient.h"

namespace BaeServer
{
	class ChannelPool
	{
		using self_type = ChannelPool;
		using container_channel = std::map<Channel::id_type, Channel::ptr>;
		using container_member = std::map<TalkToClient::id_type, Channel::id_type>;
		
		//non copyable
		self_type(const self_type & ac) = delete;
		self_type & operator=(const self_type & ac) = delete;

		//non movable
		self_type(const self_type && ac) = delete;
		self_type & operator=(const self_type && ac) = delete;

		ChannelPool() : id_flag_{1}, threads_per_channel(DEFAULT_NUM_THREADS_PER_CHANNEL) {}

	public:
		static const size_t DEFAULT_NUM_THREADS_PER_CHANNEL = 2;// at least 2 required (for deadline_timer)
		//using ptr = std::unique_ptr<ChannelPool>;
		using ptr = ChannelPool *;

		static ptr GetInstance()
		{
			if (nullptr == instance) { 
				instance = ptr{new ChannelPool()}; 
			}
			
			return instance;
		}

		static void SetNumThreadsPerChannel(size_t threadNum)
		{
			GetInstance()->threads_per_channel = { threadNum };
		}

		Channel::ptr GetCapableChannel(unsigned short base_port);
		void AddToChannel(TalkToClient::ptr client, Channel::id_type id);
		void FireDuplicateUser(TalkToClient::id_type id);
	private:
		static ptr instance;

		size_t id_flag_;
		size_t threads_per_channel;
		container_channel channels_;
		container_member members_;
		std::mutex mutex_;
	};
}