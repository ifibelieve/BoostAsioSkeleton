#include "TalkToServer.h"
#include <boost/asio/yield.hpp>
#include "flatbuffers/flatbuffers.h"
#include "Protocols/ReqLogin_generated.h"
#include "Protocols/InfoChannel_generated.h"
#include "Protocols/AnsLogin_generated.h"
#include "Protocols/ReqPing_generated.h"
#include "Protocols/AnsPing_generated.h"

void TalkToServer::Start(ip::tcp::endpoint ep)
{
	Log("Start");
	sock_.async_connect(ep, MEM_FN3(RunCycle, _1, 0));
}

TalkToServer::ptr TalkToServer::Start(io_service &service, id_type id, ip::tcp::endpoint ep)
{
	ptr conn{ new TalkToServer(service, id) };
	conn->Start(ep);
	return conn;
}

void TalkToServer::RunCycle(const boost::system::error_code & ec, size_t bytes)
{
	reenter(this) {
		for (;;) {
			if (false == started_)
			{
				if (ec) { yield break; }

				ReadyReqLogin();
				started_ = { true };
			}
			Log("async write ec:" + ec.value());
			yield async_write(sock_, write_buff_, MEM_FN3(RunCycle, _1, _2));
			//write_buff_.commit(write_buff_.size());
			if (ec) { yield break; }

			Log("async read ec:" + ec.value());
			yield async_read_until(sock_, read_buff_, PACKET_END, MEM_FN3(RunCycle, _1, _2));
			if (ec) { yield break; }

			Log("HandleAnswer ec:" + ec.value());
			yield service_.post(MEM_FN3(HandleAnswer, ec, bytes));
			if (ec) { yield break; }
		}
	}
}

void TalkToServer::HandleAnswer(const boost::system::error_code &ec, size_t bytes)
{
	Log("HandleAnswer");

	if (read_buff_.size() > 0)
	{
		const uint8_t * cptr =
		{ buffer_cast<const uint8_t *>(read_buff_.data()) };

		flatbuffers::Verifier verifier{ cptr, bytes };

		std::string answer;
		std::getline(std::istream(&read_buff_), answer);
		read_buff_.consume(read_buff_.size());

		size_t blankIndex = answer.find(" ");
		if (blankIndex > answer.size())
		{
			Log("invalid answer format " + answer, std::cerr);
			return;
		}

		std::string name{ answer, blankIndex + 1 };
		std::string fb{ answer, 0, blankIndex - 1 };

		if (0 == name.find("AnsLogin"))// && Protocols::VerifyAnsLoginBuffer(verifier))
		{
			OnAnsLogin(fb);
		}
		else if (0 == name.find("AnsPing"))// && Protocols::VerifyAnsPingBuffer(verifier))
		{
			OnPing(fb);
		}
		else
		{
			std::cerr << "invalid answer" << std::endl;
			return;
		}
	}

	if (write_buff_.size() > 0)
	{
		service_.post(
			MEM_FN3(RunCycle, boost::system::error_code(), 0));
	}
}

void TalkToServer::ReadyReqLogin()
{
	Log("ReadyReqLogin");

	flatbuffers::FlatBufferBuilder builder;
	auto reqLogin = Protocols::CreateReqLogin(builder, id_, 1);
	builder.Finish(reqLogin);

	uint8_t * byte = builder.GetBufferPointer();
	std::string s{ byte, byte + builder.GetSize() };

	std::ostream os(&write_buff_);
	os << s << " ReqLogin" << PACKET_END;
}

void TalkToServer::OnAnsLogin(std::string answer)
{
	Log("OnAnsLogin");

	flatbuffers::FlatBufferBuilder builder;
	auto ansLogin = Protocols::GetAnsLogin(answer.c_str());

	id_type	userId = ansLogin->id();

	if (NULL != ansLogin->channel())
	{
		channel_id_ = ansLogin->channel()->id();
	}

	PostponePing();
}

void TalkToServer::PostponePing()
{
	Log("PostPonePing");

	timer_.expires_from_now(boost::posix_time::milliseconds(PING_MILLISEC));
	timer_.async_wait(MEM_FN1(DoPing));
}

void TalkToServer::DoPing()
{
	Log("DoPing");

	flatbuffers::FlatBufferBuilder builder;
	auto reqPing = Protocols::CreateReqPing(builder, id_);
	builder.Finish(reqPing);

	uint8_t * byte = builder.GetBufferPointer();
	std::string s(byte, byte + builder.GetSize());

	std::ostream os{ &write_buff_ };
	os << s << " ReqPing" << PACKET_END;

	service_.post(MEM_FN3(RunCycle, boost::system::error_code(), 0));
}

void TalkToServer::OnPing(std::string answer)
{
	Log("OnPing");

	flatbuffers::FlatBufferBuilder builder;
	auto ansPing = Protocols::GetAnsPing(answer.c_str());

	id_type	userId = ansPing->id();

	if (NULL != ansPing->channel())
	{
		channel_id_ = ansPing->channel()->id();
	}

	PostponePing();
}

void TalkToServer::Log(std::string msg, std::ostream & os)
{
	os << "[TalkToServer" << id_ << "] channel:" << channel_id_ 
		<< " " << msg << std::endl;
}