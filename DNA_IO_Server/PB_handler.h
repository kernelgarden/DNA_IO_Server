#pragma once
#include <iostream>
#include <string>

#include <boost/function.hpp>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/text_format.h>

#include "protocol.pb.h"

using namespace google;

#pragma pack(push, 1)
// 메시지의 헤더
struct PacketHeader
{
	protobuf::uint32 size;
	dna_info::packet_type type;
};
#pragma pack(pop)

class GameServer;

/*
* protobuf를 사용한 패킷들을 테스트하는 클래스
*/
class PacketHandler
{
public:
	PacketHandler() { }
	PacketHandler(GameServer *p_server)
		: m_server(p_server)
	{

	}

	void Init(GameServer *p_server)
	{
		m_server = p_server;
	}

	/* 서버로 보내는 로그인 요청 패킷 */
	void Handle(const dna_info::LoginRequest& message, int p_session_id) const;
	/*
	void Handle(const dna_info::LoginRequest& message) const
	{
		PrintMessage(message);
		//std::cout << m_server->m_bIsAccepting;
	}
	*/

	void Handle(const dna_info::LoginResponse& message) const
	{
		PrintMessage(message);
	}

	void Handle(const dna_info::SyncInfo_C& message) const
	{
		PrintMessage(message);
	}

	void Handle(const dna_info::SyncInfo_S& message) const
	{
		PrintMessage(message);
	}
	 
	void Handle(const dna_info::UserInfo& message) const
	{
		PrintMessage(message);
	}

protected:
	/*
	* 메시지를 표준 출력으로 출력합니다.
	*/
	void PrintMessage(const protobuf::Message& message) const
	{
		std::string textFormatStr;
		protobuf::TextFormat::PrintToString(message, &textFormatStr);
		std::cout << textFormatStr.c_str() << std::endl;
	}
private:
	GameServer *m_server;
};

/*
* 스트림으로부터 패킷을 받아서 메시지에 따라 처리를 해줍니다.
*/
int Process_packet(protobuf::io::CodedInputStream& input_stream, const PacketHandler& handler, int p_session_id=-1);

/*
* 패킷 메시지를 스트림으로 씁니다.
*/
void WriteMessageToStream(const protobuf::Message& message,
	dna_info::packet_type message_type, protobuf::io::CodedOutputStream& stream);