#pragma once

#include "TalkToClient.h"

class Channel :
	public boost::enable_shared_from_this<Channel>,
	boost::noncopyable
{
	typedef Channel self_type;

#define CHANNEL_CAPACITY 2
#define DEFAULT_PORT     9000
#define MEM_FN(x) boost::bind(&self_type::x, shared_from_this())
	Channel(size_t id) :
		id_(id),
		ep_(ip::tcp::v4(), DEFAULT_PORT),
		ac_(service_, ep_)
	{}
public:
	typedef boost::shared_ptr<self_type> ptr;

	static ptr NewPtr(size_t id) { return ptr(new Channel(id)); }
	
	bool HasRoom() const;
	void AddClient(TalkToClient::ptr &client);
	void Start(size_t threadNum);

	io_service & get_service() { return service_; }
	ip::tcp::endpoint & get_endpoint() { return ep_; }
	ip::tcp::acceptor & get_acceptor() { return ac_; }

	size_t get_id() const { return id_; }
private:
	void RunService();
private:
	io_service service_; //for separated select pool in linux
	ip::tcp::endpoint ep_;
	ip::tcp::acceptor ac_;

	size_t id_;
	bool started_;
	bool member_changed_;
	std::map<std::string, TalkToClient::ptr> member_map_;
	boost::thread_group threads_;
	boost::recursive_mutex mutex_start_;
	boost::recursive_mutex mutex_member_;
};

