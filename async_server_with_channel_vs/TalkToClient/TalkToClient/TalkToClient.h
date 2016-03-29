#pragma once

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "flatbuffers/flatbuffers.h"

using namespace boost::asio;

class TalkToClient :
	public boost::enable_shared_from_this<TalkToClient>,
	boost::noncopyable
{
	typedef TalkToClient self_type;

#define PACKET_END "*_bae_\n"
#define EXPIRED_MILLISEC 10000

#define MEM_FN1(x)		boost::bind(&self_type::x, shared_from_this())
#define MEM_FN2(x,y)	boost::bind(&self_type::x, shared_from_this(), y)
#define MEM_FN3(x,y,z)	boost::bind(&self_type::x, shared_from_this(), y, z)

	TalkToClient(io_service &service) : 
		sock_(service), timer_(service), started_(false),
		id_(0), channel_id_(0) {}
public:
	typedef unsigned long long id_type;
	typedef boost::shared_ptr<self_type> ptr;

	static ptr New(io_service &service) { return ptr(new self_type(service)); }
	ip::tcp::socket & sock() { return sock_; }

	void Start()
	{
		std::cout << "[TalkToClient:" << id_ << "] Start thread:" << boost::this_thread::get_id() << std::endl;

		started_ = true;
		//wait login
		DoRead();
	}

	void Stop()
	{
		std::cout << "[TalkToClient:" << id_ << "] Stop thread:" << boost::this_thread::get_id() << std::endl;

		if (false == started_)
		{
			return;
		}

		started_ = false;
		sock_.close();
	}

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
private:
	ip::tcp::socket sock_;
	deadline_timer timer_;
	bool started_;
	id_type id_;
	id_type channel_id_;
	streambuf read_buff_;
	streambuf write_buff_;
	boost::mutex cs_;
	boost::posix_time::ptime last_time_;
};

