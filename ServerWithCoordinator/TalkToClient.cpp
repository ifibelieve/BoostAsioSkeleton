#include "TalkToClient.h"
#include "../Protocols/AnsLogin_generated.h"
#include "../Protocols/AnsPing_generated.h"
#include "../Protocols/InfoChannel_generated.h"
#include "../Protocols/ReqLogin_generated.h"
#include "../Protocols/ReqPing_generated.h"
#include <iostream>

using namespace BaeServer;

void TalkToClient::Start()
{
	Log("Start");

	started_ = true;
	//wait login
	DoRead();
}

void TalkToClient::Stop()
{
	Log("Stop");

	if (false == started_)
	{
		return;
	}

	started_ = false;
	sock_.close();
}

void TalkToClient::DoRead()
{
	Log("DoRead");
	async_read_until(sock_, read_buff_, PACKET_END, MEM_FN3(OnRead, _1, _2));
	CheckTimeOut();

	last_time_ = boost::posix_time::microsec_clock::local_time();
	//last_time_ = std::chrono::high_resolution_clock::now();
}

void TalkToClient::OnRead(const boost::system::error_code &ec, size_t bytes)
{
	Log("OnRead ec:" + ec.value());

	//connection closed
	if (ec)
	{
		if (error::eof == ec)
		{
			Log("connection is closed");
		}

		Stop();
		return;
	}

	std::string request;
	std::getline(std::istream(&read_buff_), request);

	size_t blankIndex = request.find(" ");
	if (blankIndex > request.size())
	{
		return;
	}

	std::string name(request, blankIndex + 1);
	std::string fb(request, 0, blankIndex - 1);

	const uint8_t * cptr =
		buffer_cast<const uint8_t *>(read_buff_.data());

	flatbuffers::Verifier verifier(cptr, bytes - name.size() - 1);

	HandleRequest(verifier, name, fb);
}

void TalkToClient::HandleRequest(flatbuffers::Verifier & verifier, std::string name, std::string fb)
{
	if (0 == name.find("ReqPing"))// && Protocols::VerifyReqPingBuffer(verifier))
	{
		OnPing(fb);
	}
	else if (0 == name.find("ReqLogin"))// && Protocols::VerifyReqLoginBuffer(verifier))
	{
		OnLogin(fb);
	}
	else
	{
		Log("invalid request", std::cerr);
	}
}

void TalkToClient::DoWrite(std::string response)
{
	Log("DoWrite");
	
	std::ostream os(&write_buff_);
	os << response << PACKET_END;

	async_write(sock_, write_buff_, MEM_FN3(OnWrite, _1, _2));
}

void TalkToClient::OnWrite(const boost::system::error_code &ec, size_t bytes)
{
	Log("OnWrite");

	if (ec)
	{
		Log("OnWrite ec:" + ec.value(), std::cerr);
	}

	DoRead();
}

void TalkToClient::CheckTimeOut()
{
	timer_.expires_from_now(boost::posix_time::milliseconds(EXPIRED_MILLISEC));
	timer_.async_wait(MEM_FN1(OnCheckTimeOut));
}

void TalkToClient::OnCheckTimeOut()
{
	Log("OnCheckTimeOut");

	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	if ((now - last_time_).total_milliseconds() > EXPIRED_MILLISEC)
	{
		Log("expired", std::cerr);
		Stop();
	}

	last_time_ = now;
}

void TalkToClient::OnLogin(std::string request)
{
	auto reqLogin = Protocols::GetReqLogin(request.c_str());

	id_ = reqLogin->id();
	
	/* TODO replace ChannelPool
	ChannelPool::GetInstance()->FireDuplicateUser(id_);
	ChannelPool::GetInstance()->AddToChannel(
		shared_from_this(), channel_id_);
	*/

	Log("OnLogin");

	flatbuffers::FlatBufferBuilder builder;
	auto infoChannel = Protocols::CreateInfoChannel(builder, channel_id_);
	auto ansLogin = Protocols::CreateAnsLogin(builder, id_, infoChannel);
	builder.Finish(ansLogin);

	uint8_t * byte = builder.GetBufferPointer();
	std::string fb(byte, byte + builder.GetSize());

	DoWrite(fb + " AnsLogin");
}

void TalkToClient::OnPing(std::string request)
{
	Log("OnPing");

	auto reqPing = Protocols::GetReqPing(request.c_str());
	std::cerr << "ping from id:" << reqPing->id() << std::endl;

	flatbuffers::FlatBufferBuilder builder;
	auto infoChannel = Protocols::CreateInfoChannel(builder, channel_id_);
	auto ansPing = Protocols::CreateAnsPing(builder, id_, infoChannel);
	builder.Finish(ansPing);

	uint8_t * byte = builder.GetBufferPointer();
	std::string fb(byte, byte + builder.GetSize());

	DoWrite(fb + " AnsPing");
}

void TalkToClient::Log(std::string msg, std::ostream & os)
{
	os << "[TalkToClient" << id_ << "] channel:" << channel_id_
		<< " " << msg << std::endl;
}
