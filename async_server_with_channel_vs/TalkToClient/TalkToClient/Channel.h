#pragma once

#include "TalkToClient.h"

class Channel :
	public boost::enable_shared_from_this<Channel>,
	boost::noncopyable
{
	typedef Channel self_type;

#define CHANNEL_CAPACITY 2
#define MEM_FN(x) boost::bind(&self_type::x, shared_from_this())
	Channel(size_t id, unsigned short port) :
		id_(id),
		ep_(ip::tcp::v4(), port),
		ac_(service_, ep_),
		started_(false)
	{
		/*ac_.open(ep_.protocol());
		ac_.set_option(ip::tcp::acceptor::reuse_address(true));
		ac_.bind(ep_);
		ac_.listen();*/
	}
public:
	typedef unsigned long long id_type;
	typedef boost::shared_ptr<self_type> ptr;

	static ptr NewPtr(size_t id, unsigned short port) { return ptr(new Channel(id, port)); }
	
	bool HasRoom();
	void AddClient(TalkToClient::ptr &client);
	void Start(size_t threadNum);

	io_service & get_service() { return service_; }
	ip::tcp::endpoint & get_endpoint() { return ep_; }
	ip::tcp::acceptor & get_acceptor() { return ac_; }

	size_t id() const { return id_; }
private:
	void DoAccept();
	void OnAccept(TalkToClient::ptr conn, const boost::system::error_code & ec);
private:
	io_service service_; //for separated select pool in linux
	ip::tcp::endpoint ep_;
	ip::tcp::endpoint remote_ep_;
	ip::tcp::acceptor ac_;

	size_t id_;
	bool started_;
	std::map<TalkToClient::id_type, TalkToClient::ptr> member_map_;
	boost::thread_group threads_;
	boost::recursive_mutex mutex_start_;
	boost::recursive_mutex mutex_member_;
};

