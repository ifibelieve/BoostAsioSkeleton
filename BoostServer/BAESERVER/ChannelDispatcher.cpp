#include "ChannelDispatcher.h"
#include "ChannelPool.h"

#include <thread>
#include <exception>
#include <boost/bind.hpp>

using namespace BaeServer;

void BaeServer::ChannelDispatcher::Run(io_service & service, unsigned short port)
{
	ptr conn = ChannelDispatcher::New(service, port);
	conn->Start();
}

BaeServer::ChannelDispatcher::ptr BaeServer::ChannelDispatcher::New(io_service & service, unsigned short port)
{
	return ptr(new self_type{ service, port });
}

void BaeServer::ChannelDispatcher::Start()
{
	DoAccept();
}

void BaeServer::ChannelDispatcher::Stop()
{
	sock_.close();
}

void BaeServer::ChannelDispatcher::DoAccept()
{
	ac_.async_accept(sock_, remote_ep_, 
		boost::bind(&ChannelDispatcher::OnAccept, shared_from_this(), _1));
}

void BaeServer::ChannelDispatcher::OnAccept(const boost::system::error_code & ec)
{
	if (ec) std::cerr << "[ChannelDispatcher::OnAccept] ec:" << ec << std::endl;

	SendChannelAddress();
}

void BaeServer::ChannelDispatcher::SendChannelAddress()
{
	Channel::ptr channel = ChannelPool::GetInstance()->GetCapableChannel(ep_.port());
	ip::tcp::endpoint ep(sock_.local_endpoint().address(), channel->get_endpoint().port());

	std::string epString = ep.address().to_string() + " " + std::to_string(ep.port()) + "\n";

	std::copy(epString.begin(), epString.end(), write_buf);
	async_write(sock_, buffer(write_buf, epString.size()),
		boost::bind(&ChannelDispatcher::OnSend, shared_from_this(), _1, _2));
}

void BaeServer::ChannelDispatcher::OnSend(const boost::system::error_code & ec, size_t bytes)
{
	std::cout << "[ChannelDispatcher::OnSend] ec:" << ec << " bytes:" << bytes << std::endl;
	//Stop();
	ChannelDispatcher::Run(service_, ep_.port());
}
