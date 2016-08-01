// BoostClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <exception>
#include <random>
#include <boost/thread.hpp>
#include "TalkToServer.h"

ip::tcp::endpoint GetChannelAddress();
TalkToServer::id_type GetId();
size_t OnReadComplete(char * buf, const boost::system::error_code & ec, size_t bytes);

int main()
{
	try
	{
		io_service ch_service;

		ip::tcp::endpoint ep = GetChannelAddress();
		std::cout << "channel address " << ep.address() << ":" << ep.port() << std::endl;
		
		TalkToServer::id_type id = GetId();

		TalkToServer::ptr conn = TalkToServer::Start(ch_service, id, ep);
		ch_service.run(); //use coroutine
	}
	catch (std::exception & e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

size_t OnReadComplete(char * buf, const boost::system::error_code & ec, size_t bytes)
{
	if (ec) return 0;

	bool result = std::find(buf, buf + bytes, '\n') < buf + bytes;

	return result ? 0 : 1;
}

TalkToServer::id_type GetId()
{
	TalkToServer::id_type id{};

	std::cout << "enter id" << std::endl;
	std::cin >> id;

	if (0 == id)
	{
		auto rand { std::bind(std::uniform_int_distribution<>{ 1, 1000 }, std::default_random_engine{ (unsigned int)time(0) }) };
		id = rand();
	}

	return id;
}

ip::tcp::endpoint GetChannelAddress()
{
	io_service base_service;
	ip::tcp::endpoint base_ep(ip::address::from_string("127.0.0.1"), 9000);
	char buf[1024];

	std::string answer;

	boost::system::error_code ec;
	while (true)
	{
		ip::tcp::socket sock(base_service);
		sock.connect(base_ep, ec);

		if (ec) break;

		size_t bytes = read(sock, buffer(buf),
			boost::bind(OnReadComplete, buf, _1, _2), ec);

		if (ec) break;

		answer = { buf, bytes };
		break;
	}

	if (ec)
	{
		throw std::exception("fail to get channel address ec:" + ec.value());
	}

	std::vector<std::string> address_port;
	std::stringstream ss(answer);
	std::string token;
	while (ss >> token)
	{
		address_port.push_back(token);
	}

	if (address_port.size() != 2)
	{
		throw std::exception("invalid channel address");
	}

	ip::tcp::endpoint ep(
		ip::address::from_string(address_port[0]),
		atoi(address_port[1].c_str()));

	return ep;
}
