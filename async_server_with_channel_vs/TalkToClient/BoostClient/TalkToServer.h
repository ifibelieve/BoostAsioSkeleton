#pragma once

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

using namespace boost::asio;

class TalkToServer :
	public boost::enable_shared_from_this<TalkToServer>,
	public coroutine,
	boost::noncopyable
{
	typedef TalkToServer self_type;
	typedef unsigned long long id_type;

#define PACKET_END "*_bae_\n"
#define PING_MILLISEC 5000

#define MEM_FN1(x)			boost::bind(&self_type::x, shared_from_this())
#define MEM_FN2(x, y)		boost::bind(&self_type::x, shared_from_this(), y)
#define MEM_FN3(x, y, z)	boost::bind(&self_type::x, shared_from_this(), y, z)

	TalkToServer(io_service &service, id_type id) : 
		service_(service), sock_(service), timer_(service), 
		id_(id), started_(false) {}

	void Start(ip::tcp::endpoint ep)
	{
		std::cout << "[TalkToServer" << id_ << "] Start thread:" << boost::this_thread::get_id() << std::endl;
		sock_.async_connect(ep, MEM_FN3(RunCycle, _1, 0));
	}
public:
	typedef boost::shared_ptr<self_type> ptr;

	static ptr Start(io_service &service, id_type id, ip::tcp::endpoint ep)
	{
		ptr conn(new TalkToServer(service, id));
		conn->Start(ep);
		return conn;
	}

private:
	void RunCycle(const boost::system::error_code &ec, size_t bytes);
	void HandleAnswer(const boost::system::error_code &ec, size_t bytes);

	void ReadyReqLogin();
	void OnAnsLogin(std::string);

	void PostponePing();
	void DoPing();
	void OnPing(std::string);
private:
	bool started_;
	id_type id_;
	id_type channel_id_;
	io_service &service_;
	ip::tcp::socket sock_;
	deadline_timer timer_;
	streambuf read_buff_;
	streambuf write_buff_;
};

