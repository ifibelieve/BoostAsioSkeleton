// TalkToClient.cpp : Defines the entry point for the console application.
//

#include "TalkToClient.h"
#include "ChannelPool.h"
#include "Acceptor.h"

#define BASE_PORT			8800
#define CONNECT_THREAD_NUM  3
#define CHANNEL_THREAD_NUM	4// at least 2 required (for deadline_timer)

using namespace boost::asio;

void DoAccept();
void OnAccept(Acceptor::ptr acceptor, const boost::system::error_code & ec);
void OnSendChannelAddress(Acceptor::ptr acceptor, const boost::system::error_code & ec, size_t bytes);

io_service service;

void DoAccept()
{
	std::cout << "[BASE] DoAccept thread:" << boost::this_thread::get_id() << std::endl;

	Acceptor::ptr acceptor = Acceptor::New(service, BASE_PORT);
	acceptor->AsyncAccept(boost::bind(OnAccept, acceptor, _1));
}

void OnAccept(Acceptor::ptr acceptor, const boost::system::error_code & ec)
{
	std::cout << "[BASE] OnAccept ec:" << ec << " thread:" << boost::this_thread::get_id() << std::endl;

	if (ec)
	{
		DoAccept();
		return;
	}

	Channel::ptr channel = 
		ChannelPool::GetInstance()->GetCapableChannel(BASE_PORT, CHANNEL_THREAD_NUM);

	unsigned short port = channel->get_endpoint().port();
	acceptor->AsyncSendPort(port, boost::bind(OnSendChannelAddress, acceptor, _1, _2));
}

void OnSendChannelAddress(Acceptor::ptr acceptor, const boost::system::error_code & ec, size_t bytes)
{
	std::cout << "[BASE] OnSendChannelAddress ec:" << ec << " thread:" << boost::this_thread::get_id() << std::endl;

	acceptor->Stop();
	DoAccept();
}

int main()
{
	DoAccept();

	boost::thread_group threads;
	for (size_t i = 0; i < CONNECT_THREAD_NUM; i++)
	{
		boost::thread * thread = threads.create_thread(boost::bind(&io_service::run, &service));
		std::cout << "[BASE] thread:" << thread->get_id() << std::endl;
	}

	threads.join_all();

	return 0;
}

