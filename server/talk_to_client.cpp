//#define BOOST_ASIO_ENABLE_HANDLER_TRACKING

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

using namespace boost::asio;

class talk_to_client :
	public boost::enable_shared_from_this<talk_to_client>,
	boost::noncopyable
{
	typedef talk_to_client self_type;

#define MEM_FN1(x) boost::bind(&self_type::x, shared_from_this())
#define MEM_FN2(x, y) boost::bind(&self_type::x, shared_from_this(), y)
#define MEM_FN3(x, y, z) boost::bind(&self_type::x, shared_from_this(), y, z)

	talk_to_client(io_service &service, ip::tcp::acceptor & ac, const ip::tcp::endpoint &ep) : service_(service), sock_(service_), ac_(ac)
	{
		//sock_.open(ip::tcp::v4());
		//sock_.bind(ep); // bind: Address already in use
		//ac_ = ip::tcp::acceptor(service, ep);
	}
public:
	typedef boost::system::error_code error_code;
	typedef boost::shared_ptr<self_type> ptr;

	static void start(io_service &service, ip::tcp::acceptor & ac, ip::tcp::endpoint &bind_ep)
	{
		ptr new_(new talk_to_client(service, ac, bind_ep));
		new_->ac().async_accept(new_->sock(), boost::bind(&talk_to_client::step, new_, _1, 0));
	}

	static ptr new_(io_service &service, ip::tcp::acceptor & ac, const ip::tcp::endpoint &bind_ep)
	{
		ptr new_(new talk_to_client(service, ac, bind_ep));
		return new_;
	}

	static ptr new_(ip::tcp::socket & sock, ip::tcp::acceptor & ac)
	{
		ptr new_(new talk_to_client(sock.get_io_service(), ac, sock.local_endpoint()));
		return new_;
	}

	void start()
	{
		ac_.async_accept(sock_, MEM_FN3(step, _1, 0));
	}

	void step(error_code ec, size_t bytes)
	{
		std::cout << boost::this_thread::get_id() << std::endl;
		for (;;)
		{
		}
	}

	ip::tcp::socket& sock() { return sock_; }
	ip::tcp::acceptor& ac() { return ac_; }
private:
	io_service& service_;
	ip::tcp::socket sock_;
	//ip::tcp::acceptor ac_; change to use global acceptor
	ip::tcp::acceptor & ac_;
};

void on_accept(talk_to_client::ptr ttc, const boost::system::error_code &ec)
{
	ttc->step(ec, 0);
	
	talk_to_client::ptr new_ = talk_to_client::new_(ttc->sock(), ttc->ac());
	new_->ac().async_accept(new_->sock(), boost::bind(on_accept, new_, _1));
	
}

int main(int argc, char **argv)
{
	io_service service;
	ip::tcp::endpoint ep(ip::tcp::v4(), 9000);
	ip::tcp::acceptor ac(service, ep);

	talk_to_client::ptr new_ = talk_to_client::new_(service, ac, ep);
	//socket_base::reuse_address option(true);
	//new_->ac().set_option(option);
	new_->ac().async_accept(new_->sock(), boost::bind(on_accept, new_, _1));

	service.run();
	return 0;
}
