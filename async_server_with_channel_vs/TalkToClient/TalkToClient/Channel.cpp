#include "Channel.h"

bool Channel::HasRoom()
{
	bool result = false;

	{
		boost::recursive_mutex::scoped_lock lk(mutex_member_);
		result = member_map_.size() < CHANNEL_CAPACITY;
	}

	return result;
}

void Channel::AddClient(TalkToClient::ptr & client)
{
	{
		boost::recursive_mutex::scoped_lock lk(mutex_member_);
		member_map_.insert_or_assign(client->id(), client);
		client->set_channel_id(id_);
	}
}

void Channel::Start(size_t thread_num)
{
	{
		boost::recursive_mutex::scoped_lock lk(mutex_start_);
		if (started_)
		{
			return;
		}
		started_ = true;
	}

	DoAccept();

	for (size_t i = 0; i < thread_num; i++)
	{
		boost::thread * thread = threads_.create_thread(boost::bind(&io_service::run, &service_));
		std::cout << "channel:" << id_ << " thread id:" << thread->get_id() << std::endl;
	}
}

void Channel::DoAccept()
{
	std::cerr << "[Channel] do accept thread:" << boost::this_thread::get_id() << std::endl;
	
	TalkToClient::ptr client = TalkToClient::New(service_);

	std::cout << "[Channel] channel:" << id_ << " port:" << ep_.port() << std::endl;

	ac_.async_accept(
		client->sock(),
		remote_ep_,
		MEM_FN3(OnAccept, client, _1));
}

void Channel::OnAccept(TalkToClient::ptr conn, const boost::system::error_code & ec)
{
	std::cerr << "[Channel] on accept ec:" << ec << " thread:" << boost::this_thread::get_id() << std::endl;

	if (0 == ec)
	{
		conn->set_channel_id(id_);
		//handle in service_'s threads
		service_.post(boost::bind(&TalkToClient::Start, conn));
	}

	DoAccept();
}
