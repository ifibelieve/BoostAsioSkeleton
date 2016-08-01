#pragma once

#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;

namespace BaeServer
{
	class ChannelDispatcher :
		public std::enable_shared_from_this<ChannelDispatcher>
	{
		using self_type = ChannelDispatcher;
		using ptr = std::shared_ptr<self_type>;

		//non copyable
		self_type(const self_type & ac) = delete;
		self_type & operator=(const self_type & ac) = delete;

		ChannelDispatcher(io_service & service, unsigned short port) :
			service_{ service }, sock_{ service_ },
			ep_{ ip::tcp::v4(), port }, ac_{ service_, ep_ } {}
	public:
		static void Run(io_service & service, unsigned short port);

		void Start();
		void Stop();
	private:
		static ptr New(io_service & service, unsigned short port);

		void DoAccept();
		void OnAccept(const boost::system::error_code & ec);

		void SendChannelAddress();
		void OnSend(const boost::system::error_code & ec, size_t bytes);
	private:
		io_service & service_;
		ip::tcp::endpoint ep_;
		ip::tcp::endpoint remote_ep_;
		ip::tcp::acceptor ac_;
		ip::tcp::socket sock_;
		enum { msg_max = 1024 };
		char read_buf[msg_max];
		char write_buf[msg_max];
	};
}
