#include "Channel.h"
#include <mutex>

using namespace BaeServer;
using namespace std;

bool Channel::HasRoom()
{
	bool result = false;

	//lock lk{ mutex_member_ };
	result = member_container_.size() < CHANNEL_CAPACITY;
	
	return result;
}

void Channel::AddClient(TalkToClient::ptr & client)
{
	type_lock lk{ mutex_member_ };
	if (0 == member_container_.count(client->id())) {
		member_container_.insert(
			std::pair<TalkToClient::id_type, TalkToClient::ptr>(client->id(), client));
	} else {
		member_container_[client->id()] = client;
	}
	lk.unlock();

	client->set_channel_id(id_);
}

void BaeServer::Channel::FireClient(TalkToClient::id_type id)
{
	auto it = member_container_.find(id);
	if (member_container_.end() != it)
	{
		it->second->Stop();
		member_container_.erase(it);
	}
}

void Channel::Run(size_t port, size_t thread_num)
{
	auto channel = Channel::New(port);
	channel->Start();	

	boost::thread_group threads;

	for (size_t i = 0; i < thread_num; i++)
	{
		boost::thread * thread = threads.create_thread(boost::bind(&Channel::RunService, channel));
	}
	
	threads.join_all();
}

void Channel::Start() 
{
	type_lock lk{ mutex_start_ };
        if (started_)
        {
                return;
        }
        started_ = true;
        lk.unlock();

        DoAccept();
}

void Channel::RunService()
{
	service_.run();
}

void Channel::Init() 
{
	InitZookeeper();
}

void Channel::InitZookeeper()
{
	zk_ = std::shared_ptr<ZkHandler>(new ZkHandler());
        zk_->CreateGroup("session");

	std::string name = GetIpAddress() + ":" + std::to_string(ep_.port());
	zk_->CreateMember(name, "0");
}

std::string Channel::GetIpAddress() {
	ip::tcp::resolver resolver(service_);
	ip::tcp::resolver::query query(ip::tcp::v4(), ip::host_name(), "");
	ip::tcp::resolver::iterator iter = resolver.resolve(query);
	ip::tcp::endpoint ep = *iter;

	return move(ep.address().to_string());
}

void Channel::DoAccept()
{
	std::cerr << "[Channel] do accept thread:" << std::this_thread::get_id() << std::endl;

	TalkToClient::ptr client { TalkToClient::New(service_) };

	std::cout << "[Channel] channel:" << id_ << " port:" << ep_.port() << std::endl;

	ac_.async_accept(
		client->sock(),
		remote_ep_,
		MEM_FN3(OnAccept, client, _1));
}

void Channel::OnAccept(TalkToClient::ptr conn, const boost::system::error_code & ec)
{
	std::cerr << "[Channel] on accept ec:" << ec << " thread:" << std::this_thread::get_id() << std::endl;

	if (0 == ec)
	{
		conn->set_channel_id(id_);
		//handle in service_'s threads
		service_.post(boost::bind(&TalkToClient::Start, conn));
	}

	DoAccept();
}
