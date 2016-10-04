#include "coordinator.h"
#include "boost/bind.hpp"
#include <sstream>

using namespace NSCoordinator;
using namespace std;
using namespace boost::asio;

void Coordinator::Run(size_t port) {
	ptr conn = Coordinator::New(port);
	conn->Start();

	conn->RunService();
}

void Coordinator::RunService() {
	service_.run();
}

void Coordinator::Init() {
	InitZookeeper();
}

void Coordinator::InitZookeeper() {
	zk_ = shared_ptr<ZkHandler>(new ZkHandler());
	zk_->CreateGroup("session");
}

void Coordinator::Start() {
	DoAccept();
}

void Coordinator::Stop() {
	sock_.close();
}

void Coordinator::DoAccept() {
	ac_.async_accept(sock_, remote_ep_,
			boost::bind(&Coordinator::OnAccept, shared_from_this(), _1));
}

void Coordinator::OnAccept(const boost::system::error_code & ec) {
	if (ec) cerr << "[Coordinator::OnAccept] ec:" << ec << endl;
	SendServerAddress();
}

void Coordinator::SendServerAddress() {
	vector<string> members = zk_->GetMembers();
	
	string msg = "empty";
	if (members.size() > 0) {
		ostringstream oss;
        	ostream_iterator<string> out_it(oss, ",");
        	std::copy(members.begin(), members.end() - 1, out_it);
		oss << members.back();
		msg = oss.str();
	}

	std::copy(msg.begin(), msg.end(), write_buf_);
	async_write(sock_, boost::asio::buffer(write_buf_, msg.size()),
			boost::bind(&Coordinator::OnSend, shared_from_this(), _1, _2));
	cerr << "[Coordinator::SendServerAddress] msg:" << msg << endl;
}

void Coordinator::OnSend(const boost::system::error_code &ec, size_t bytes) {
	if (ec) cerr << "[Coordinator::OnSend] ec:" << ec << " bytes:" << bytes << endl;
	Stop();
	Start();
}
