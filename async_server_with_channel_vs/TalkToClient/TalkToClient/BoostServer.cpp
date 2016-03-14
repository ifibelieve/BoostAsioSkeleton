// TalkToClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TalkToClient.h"
#include "ChannelPool.h"

#define CONN_THREAD_NUM 1

using namespace boost::asio;

io_service	connService;
ip::tcp::acceptor connAc(connService, ip::tcp::endpoint(ip::tcp::v4(), 9000));
ChannelPool channelPool;

void DoAccept();
void OnAccept(TalkToClient::ptr conn,
	boost::shared_ptr<ip::tcp::acceptor> ac,
	const boost::system::error_code & ec);

void DoAccept()
{
	std::cout << "do accept" << std::endl;

	Channel::ptr channel = channelPool.GetCapableChannel();
	TalkToClient::ptr client = TalkToClient::New(channel->get_service());

	//ac should alive until OnAccept
	boost::shared_ptr<ip::tcp::acceptor> ac(
		new ip::tcp::acceptor(connService, channel->get_endpoint()));

	connAc.async_accept(
		client->sock(),
		channel->get_endpoint(),
		bind(OnAccept, client, ac, _1));
}

void OnAccept(TalkToClient::ptr conn,
	boost::shared_ptr<ip::tcp::acceptor> ac,
	const boost::system::error_code & ec)
{
	std::cout << "on accept ec:" << ec << std::endl;
	conn->sock().close();
	if (0 == ec)
	{
		conn->Start();
	}

	DoAccept();
}

void RunService()
{
	connService.run();
}

int main()
{
	DoAccept();
	RunService();

	boost::thread_group threads;
	for (size_t i = 0; i < CONN_THREAD_NUM; i++)
	{
		threads.create_thread(boost::bind(RunService));
	}
	threads.join_all();

	return 0;
}

