// BoostServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <exception>
#include <boost/asio.hpp>

#include "BaeServer/ChannelDispatcher.h"
#include "BaeServer/ChannelPool.h"

using namespace boost::asio;
using namespace BaeServer;

int main(int argc, char **argv)
{
	const unsigned short BASE_PORT = 9000;

	io_service service;

	try
	{
		ChannelPool::SetNumThreadsPerChannel(ChannelPool::DEFAULT_NUM_THREADS_PER_CHANNEL);
		ChannelDispatcher::Run(service, BASE_PORT);

		service.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

    return 0;
}

