#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <memory>
#include <thread>
#include <iostream>

using namespace boost::asio;

class TalkToServer :
	public boost::enable_shared_from_this<TalkToServer>,
	public coroutine
{
	using self_type = TalkToServer;
	const char * PACKET_END = "*_bae_\n";
	const size_t PING_MILLISEC = 5000;

#define MEM_FN1(x)			boost::bind(&self_type::x, shared_from_this())
#define MEM_FN2(x, y)		boost::bind(&self_type::x, shared_from_this(), y)
#define MEM_FN3(x, y, z)	boost::bind(&self_type::x, shared_from_this(), y, z)

public:
	using id_type = unsigned long long;
	using ptr = boost::shared_ptr<self_type>;

	static ptr Start(io_service &service, id_type id, ip::tcp::endpoint ep);

private:
	TalkToServer(io_service &service, id_type id) :
		service_{ service }, sock_{ service }, timer_{ service },
		id_{ id }, started_{ false } {}

	void Start(ip::tcp::endpoint ep);
	void RunCycle(const boost::system::error_code &ec, size_t bytes);
	void HandleAnswer(const boost::system::error_code &ec, size_t bytes);

	void ReadyReqLogin();
	void OnAnsLogin(std::string);

	void PostponePing();
	void DoPing();
	void OnPing(std::string);

	void Log(std::string, std::ostream & os = std::cout);
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

