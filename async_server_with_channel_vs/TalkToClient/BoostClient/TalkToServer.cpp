#include "TalkToServer.h"
#include <boost/asio/yield.hpp>
#include "flatbuffers/flatbuffers.h"
#include "Protocols/ReqLogin_generated.h"
#include "Protocols/InfoChannel_generated.h"
#include "Protocols/AnsLogin_generated.h"
#include "Protocols/ReqPing_generated.h"
#include "Protocols/AnsPing_generated.h"
#include <iostream>

void TalkToServer::RunCycle(const boost::system::error_code & ec, size_t bytes)
{
	reenter(this) {
		for (;;) {
			if (false == started_)
			{
				if (ec) { yield break; }

				ReadyReqLogin();
				started_ = true;
			}
			std::cout << "[TalkToServer" << id_ << "] async write ec:"<< ec <<" thread:" << boost::this_thread::get_id() << std::endl;
			yield async_write(sock_, write_buff_, MEM_FN3(RunCycle, _1, _2));
			write_buff_.commit(write_buff_.size());
			
			std::cout << "[TalkToServer" << id_ << "] async read ec:"<< ec << " thread:" << boost::this_thread::get_id() << std::endl;
			yield async_read_until(sock_, read_buff_, PACKET_END, MEM_FN3(RunCycle, _1, _2));
			
			std::cout << "[TalkToServer" << id_ << "] post HandleAnser ec:" << ec << " thread:" << boost::this_thread::get_id() << std::endl;
			yield service_.post(MEM_FN3(HandleAnswer, ec, bytes));
		}
	}
}

void TalkToServer::HandleAnswer(const boost::system::error_code &ec, size_t bytes)
{
	std::cout << "[TalkToServer" << id_ << "] HandleAnswer thread:" <<boost::this_thread::get_id() << std::endl;

	if (read_buff_.size() > 0)
	{
		const uint8_t * cptr =
			buffer_cast<const uint8_t *>(read_buff_.data());

		flatbuffers::Verifier verifier(cptr, bytes);
		
		std::string answer;
		std::getline(std::istream(&read_buff_), answer);
		read_buff_.consume(read_buff_.size());

		size_t blankIndex = answer.find(" ");
		if (blankIndex > answer.size())
		{
			std::cerr << "invalid answer format " << answer << std::endl;
			return;
		}

		std::string name(answer, blankIndex + 1);
		std::string fb(answer, 0, blankIndex - 1);

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
	std::cout << "[TalkToServer" << id_ << "] ReadyReqLogin thread:" << boost::this_thread::get_id() << std::endl;

	flatbuffers::FlatBufferBuilder builder;
	auto reqLogin = Protocols::CreateReqLogin(builder, id_, 1);
	builder.Finish(reqLogin);

	uint8_t * byte = builder.GetBufferPointer();
	std::string s(byte, byte + builder.GetSize());
	
	std::ostream os(&write_buff_);
	os << s << " ReqLogin" << PACKET_END;
}

void TalkToServer::OnAnsLogin(std::string answer)
{
	std::cout << "[TalkToServer" << id_ << "] OnAnsLogin thread:" << boost::this_thread::get_id() << std::endl;

	flatbuffers::FlatBufferBuilder builder;
	auto ansLogin = Protocols::GetAnsLogin(answer.c_str());

	id_type	userId = ansLogin->id();
	
	if (NULL != ansLogin->channel())
	{
		channel_id_ = ansLogin->channel()->id();
	}

	std::cout << "id:" << userId << " channel:" << channel_id_ << std::endl;

	PostponePing();
}

void TalkToServer::PostponePing()
{
	std::cerr << "[TalkToServer" << id_ << "] PostponePing thread:" << boost::this_thread::get_id() << std::endl;

	timer_.expires_from_now(boost::posix_time::milliseconds(PING_MILLISEC));
	timer_.async_wait(MEM_FN1(DoPing));
}

void TalkToServer::DoPing()
{
	std::cerr << "[TalkToServer" << id_ << "] DoPing thread:" << boost::this_thread::get_id() << std::endl;

	flatbuffers::FlatBufferBuilder builder;
	auto reqPing = Protocols::CreateReqPing(builder, id_);
	builder.Finish(reqPing);

	uint8_t * byte = builder.GetBufferPointer();
	std::string s(byte, byte + builder.GetSize());

	std::ostream os(&write_buff_);
	os << s << " ReqPing" << PACKET_END;
	
	service_.post(MEM_FN3(RunCycle, boost::system::error_code(), 0));
	std::cerr << "id:" << id_ << " ping" << std::endl;
}

void TalkToServer::OnPing(std::string answer)
{
	std::cout << "[TalkToServer"<<id_<<"] OnPing thread:" << boost::this_thread::get_id() << std::endl;

	flatbuffers::FlatBufferBuilder builder;
	auto ansPing = Protocols::GetAnsPing(answer.c_str());

	id_type	userId = ansPing->id();

	if (NULL != ansPing->channel())
	{
		channel_id_ = ansPing->channel()->id();
	}

	std::cout << "[TalkToServer" << id_ << "] channel:" << channel_id_ << " pong" << std::endl;

	PostponePing();
}
