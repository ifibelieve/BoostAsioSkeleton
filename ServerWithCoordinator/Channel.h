#pragma once
#include <memory>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "TalkToClient.h"
#include "../Coordinator/zk_handler.h"

using namespace boost::asio;

namespace BaeServer
{
	class Channel :
		public std::enable_shared_from_this<Channel>
	{
		using self_type	= Channel;
		using container = std::map<TalkToClient::id_type, TalkToClient::ptr>;
		using type_lock = std::unique_lock<std::mutex>;
		using type_mutex = std::mutex;
		//using type_lock = boost::recursive_mutex::scoped_lock;
		//using type_mutex = boost::recursive_mutex;

		const size_t CHANNEL_CAPACITY = 2;
		
#define MEM_FN3(x,y,z)	boost::bind(&self_type::x, shared_from_this(), y, z)

		Channel(unsigned short port) :
			ep_{ ip::tcp::v4(), port },
			ac_{ service_, ep_ },
			started_{ false } { Init(); }

	public:
		using id_type = unsigned long long;
		using ptr = std::shared_ptr<self_type>;

		static void Run(size_t port, size_t threadNum);
		void RunService();

		bool HasRoom();
		void AddClient(TalkToClient::ptr &client);
		void FireClient(TalkToClient::id_type id);

		io_service & get_service() { return service_; }
		ip::tcp::endpoint & get_endpoint() { return ep_; }
		ip::tcp::acceptor & get_acceptor() { return ac_; }

		size_t id() const { return id_; }
	private:
		static ptr New(unsigned short port) { return ptr{ new Channel{port} }; }

		void Start();
		void Init();
		void InitZookeeper();
		std::string GetIpAddress();		

		void DoAccept();
		void OnAccept(TalkToClient::ptr conn, const boost::system::error_code & ec);
	private:
		io_service service_; //for separated select pool
		ip::tcp::endpoint ep_;
		ip::tcp::endpoint remote_ep_;
		ip::tcp::acceptor ac_;

		size_t id_;
		bool started_;
		container member_container_;
		boost::thread_group threads_;
		type_mutex mutex_start_;
		type_mutex mutex_member_;

		std::shared_ptr<ZkHandler> zk_;
	};
}
