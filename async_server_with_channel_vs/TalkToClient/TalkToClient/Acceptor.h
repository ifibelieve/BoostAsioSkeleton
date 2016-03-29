#pragma once
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

using namespace boost::asio;

class Acceptor :
	public boost::enable_shared_from_this<Acceptor>,
	coroutine,
	boost::noncopyable
{
	typedef Acceptor self_type;
	Acceptor(io_service & service, unsigned short port) : 
		service_(service), sock_(service_), ep_(ip::tcp::v4(), port), ac_(service_, ep_) {}
public:
	typedef boost::shared_ptr<self_type> ptr;

	static ptr New(io_service &service, unsigned short port) 
	{
		return ptr(new self_type(service, port)); 
	}
	
	template <typename AcceptHandler>
	BOOST_ASIO_INITFN_RESULT_TYPE(AcceptHandler,
		void(boost::system::error_code))
	AsyncAccept(BOOST_ASIO_MOVE_ARG(AcceptHandler) handler)
	{ 
		std::cout << "[Acceptor] thread:" << boost::this_thread::get_id() << " AsyncAccept " << std::endl;
		return ac_.async_accept(sock_, remote_ep_, handler); 
	}

	template <typename WriteHandler>
		inline BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler,
			void(boost::system::error_code, std::size_t))
	AsyncSendPort(unsigned short port, BOOST_ASIO_MOVE_ARG(WriteHandler) handler)
	{
		std::cout << "[Acceptor] thread:" << boost::this_thread::get_id() << " AsyncSendPort " << std::endl;

		std::string portString = std::to_string(port) + "\n";
		std::copy(portString.begin(), portString.end(), write_buf);
		async_write(sock_, buffer(write_buf), handler); 
	}

	void Stop()
	{
		std::cout << "[Acceptor] thread:" << boost::this_thread::get_id() << " Stop " << std::endl;
		sock_.close();
	}
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

