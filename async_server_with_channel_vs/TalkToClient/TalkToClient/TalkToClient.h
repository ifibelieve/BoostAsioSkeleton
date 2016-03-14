#pragma once

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#define EXPIRED_MICROSEC 5000

using namespace boost::asio;

class TalkToClient :
	public boost::enable_shared_from_this<TalkToClient>,
	boost::noncopyable
{
	typedef TalkToClient self_type;

#define MEM_FN1(x)		boost::bind(&self_type::x, shared_from_this())
#define MEM_FN2(x,y)	boost::bind(&self_type::x, shared_from_this(), y)
#define MEM_FN3(x,y,z)	boost::bind(&self_type::x, shared_from_this(), y, z)

	TalkToClient(io_service &service) : sock_(service), timer_(service), started_(false) {}
public:
	typedef boost::shared_ptr<self_type> ptr;

	static ptr New(io_service &service) { return ptr(new self_type(service)); }
	ip::tcp::socket & sock() { return sock_; }

	void Start()
	{
		started_ = true;
		//wait login
		DoRead();
	}

	void Stop()
	{
		if (false == started_)
		{
			return;
		}

		started_ = false;
		sock_.close();
	}
private:
	void DoRead();
	void OnRead(const boost::system::error_code &ec, size_t bytes);
	void HandleRequest(std::string request);
	
	void DoWrite(std::string response);
	void OnWrite(const boost::system::error_code &ec, size_t bytes);

	void CheckTimeOut();
	void OnCheckTimeOut();

	void UpdateClientChaged();
private:
	ip::tcp::socket sock_;
	deadline_timer timer_;
	bool started_;
	streambuf read_buff_;
	streambuf write_buff_;
	boost::mutex cs_;
	boost::posix_time::ptime last_time_;
};

