#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "zk_handler.h"

using namespace boost::asio;

namespace NSCoordinator
{
	class Coordinator :
		public std::enable_shared_from_this<Coordinator>	
	{
		using self_type = Coordinator;
		using ptr = std::shared_ptr<self_type>;

		Coordinator(size_t port) : 
			sock_(service_), ep_(ip::tcp::v4(), port), ac_(service_, ep_)
		{ Init(); }

		public:
			static void Run(size_t port);
			void RunService();
		private:
			static ptr New(size_t port) { return ptr(new self_type(port)); }
			void Init();
			void InitZookeeper();
			
			void Start();
			void Stop();

			void DoAccept();
			void OnAccept(const boost::system::error_code &ec);
			
			void SendServerAddress();
			void OnSend(const boost::system::error_code &ec, size_t bytes);

			io_service service_;
			ip::tcp::endpoint ep_;
			ip::tcp::endpoint remote_ep_;
			ip::tcp::acceptor ac_;
			ip::tcp::socket sock_;
			enum { max_msg = 1024 };
			char read_buf_[max_msg];
			char write_buf_[max_msg];
			std::shared_ptr<ZkHandler> zk_;
	};
}
