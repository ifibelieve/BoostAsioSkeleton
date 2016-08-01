#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "flatbuffers/flatbuffers.h"

using namespace boost::asio;

namespace BaeServer
{
	class TalkToClient :
		public std::enable_shared_from_this<TalkToClient>
	{
		using self_type = TalkToClient;
		using type_lock = std::unique_lock<std::mutex>;
		using type_mutex = std::mutex;
		//using type_lock = boost::recursive_mutex::scoped_lock;
		//using type_mutex = boost::recursive_mutex;
		
		const char * PACKET_END			= "*_bae_\n";
		const size_t EXPIRED_MILLISEC	= 10000;

#define MEM_FN1(x)		boost::bind(&self_type::x, shared_from_this())
#define MEM_FN2(x,y)	boost::bind(&self_type::x, shared_from_this(), y)
#define MEM_FN3(x,y,z)	boost::bind(&self_type::x, shared_from_this(), y, z)

		//non copyable
		self_type(const self_type & ac) = delete;
		self_type & operator=(const self_type & ac) = delete;

		TalkToClient(io_service &service) :
			sock_{ service }, timer_{ service }, started_{ false },
			id_{ 0 }, channel_id_{ 0 } {}
	public:
		using id_type = unsigned long long;
		using ptr = std::shared_ptr<self_type>;

		static ptr New(io_service &service) { return ptr{ new self_type(service) }; }
		ip::tcp::socket & sock() { return sock_; }

		void Start();
		void Stop();

		id_type id() { return id_; }
		void set_channel_id(id_type channel_id) { channel_id_ = channel_id; }
	private:
		void DoRead();
		void OnRead(const boost::system::error_code &ec, size_t bytes);
		void HandleRequest(flatbuffers::Verifier &, std::string, std::string);

		void DoWrite(std::string response);
		void OnWrite(const boost::system::error_code &ec, size_t bytes);

		void CheckTimeOut();
		void OnCheckTimeOut();

		void OnLogin(std::string);
		void OnPing(std::string);

		void Log(std::string, std::ostream & os = std::cout);
	private:
		ip::tcp::socket sock_;
		deadline_timer timer_;
		bool started_;
		id_type id_;
		id_type channel_id_;
		streambuf read_buff_;
		streambuf write_buff_;
		type_mutex mutex_;
		boost::posix_time::ptime last_time_;
	};
}