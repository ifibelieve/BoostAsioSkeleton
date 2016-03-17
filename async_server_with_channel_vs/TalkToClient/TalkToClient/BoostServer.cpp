// TalkToClient.cpp : Defines the entry point for the console application.
//

#include "TalkToClient.h"
#include "ChannelPool.h"

#define CHANNEL_THREAD_NUM 2

using namespace boost::asio;

ChannelPool channelPool;

void DoAccept();
void OnAccept(TalkToClient::ptr conn,
	const boost::system::error_code & ec);

void DoAccept()
{
	std::cerr << "do accept" << std::endl;

	Channel::ptr channel = channelPool.GetCapableChannel();
	TalkToClient::ptr client = TalkToClient::New(channel->get_service());

	channel->get_acceptor().async_accept(
		client->sock(),
		channel->get_endpoint(),
		bind(OnAccept, client, _1));

	channel->Start((size_t)CHANNEL_THREAD_NUM);
}

void OnAccept(TalkToClient::ptr conn,
	const boost::system::error_code & ec)
{
	std::cerr << "on accept ec:" << ec << std::endl;
	
	if (0 == ec)
	{
		conn->Start();
	}

	DoAccept();
}

int main()
{
	DoAccept();

	while (true)
	{

	}

	return 0;
}

