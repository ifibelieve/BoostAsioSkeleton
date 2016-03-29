// BoostClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "TalkToServer.h"
#include <boost/thread.hpp>

#define THREAD_NUM 3

io_service ch_service;
io_service base_service;
ip::tcp::endpoint base_ep(ip::address::from_string("127.0.0.1"), 8800);

char buf[1024];

size_t OnReadComplete(const boost::system::error_code & ec, size_t bytes)
{
	if (ec) return 0;

	bool result = std::find(buf, buf + bytes, '\n') < buf + bytes;

	return result ? 0 : 1;
}

unsigned short GetChannelPort()
{
	unsigned short port = 0;

	boost::system::error_code ec;
	while (true)
	{
		ip::tcp::socket sock(base_service);
		sock.connect(base_ep, ec);

		if (ec) break;

		read(sock, buffer(buf),
			boost::bind(OnReadComplete, _1, _2), ec);

		if (ec) break;

		port = atoi(buf);
		break;
	}

	if (ec)
	{
		std::cout << "fail to get port number ec:" << ec << std::endl;
	}

	return port;
}

int main()
{
	unsigned short port = GetChannelPort();
	std::cout << "channel address " << port << std::endl;

	ip::tcp::endpoint ch_ep(base_ep.address(), port);

	std::srand((unsigned int)std::time(NULL));
	TalkToServer::Start(ch_service, rand(), ch_ep);

	/*
	boost::thread_group threads;
	for (size_t i = 0; i < THREAD_NUM; i++)
	{
		threads.create_thread(
			boost::bind(&io_service::run, &ch_service));
	}

	threads.join_all();
	*/

	ch_service.run(); //use coroutine

	std::cout << "service is finished" << std::endl;
	std::getline(std::cin, std::string());

	return 0;

	
}

