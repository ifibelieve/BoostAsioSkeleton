#include "TalkToClient.h"

void TalkToClient::DoRead()
{
	async_read_until(sock_, read_buff_, '\n', MEM_FN3(OnRead, _1, _2));
	CheckTimeOut();

	last_time_ = boost::posix_time::microsec_clock::local_time();
}

void TalkToClient::OnRead(const boost::system::error_code &ec, size_t bytes)
{
	//connection closed
	if (error::eof == ec)
	{
		Stop();
	}

	std::string request;
	std::getline(std::istream(&read_buff_), request);

	HandleRequest(request);
}

void TalkToClient::HandleRequest(std::string request)
{
	std::cout << "request :" << request << std::endl;
}

void TalkToClient::DoWrite(std::string response)
{
}

void TalkToClient::OnWrite(const boost::system::error_code &ec, size_t bytes)
{

}

void TalkToClient::UpdateClientChaged()
{

}

void TalkToClient::CheckTimeOut()
{
	timer_.expires_from_now(boost::posix_time::milliseconds(EXPIRED_MICROSEC));
	timer_.async_wait(MEM_FN1(OnCheckTimeOut));
}

void TalkToClient::OnCheckTimeOut()
{
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	if ((now - last_time_).total_milliseconds() > EXPIRED_MICROSEC)
	{
		Stop();
	}

	last_time_ = now;
}