#include "PB_handler.h"
#include "Game_server.h"
#include "Error_code.h"

int Process_packet(protobuf::io::CodedInputStream& input_stream, const PacketHandler& handler, int p_session_id)
{
	int remain_size, nRead = 0;
	PacketHeader packet_header;

	// 스트림으로부터 헤더를 읽어옵니다.
	while (input_stream.ReadRaw(&packet_header, sizeof(PacketHeader)))
	{
		// 스트림에 직접 엑세스 할 수 있는 버퍼 포인터와 남은 길이를 알아옵니다.
		const void *payload_ptr = NULL;
		remain_size = 0;
		input_stream.GetDirectBufferPointer(&payload_ptr, &remain_size);
		if (remain_size < (signed)packet_header.size)
			break;

		nRead += packet_header.size + sizeof(PacketHeader);

		// 패킷을 읽어내기 위한 스트림을 생성합니다.
		protobuf::io::ArrayInputStream payload_array_stream(payload_ptr, packet_header.size);
		protobuf::io::CodedInputStream payload_input_stream(&payload_array_stream);

		// 패킷 본체만큼 포인터를 이동합니다.
		input_stream.Skip(packet_header.size);

		// 패킷을 분석해서 메시지 종류에 따라 처리를 합니다.
		switch (packet_header.type)
		{
		case dna_info::LOGIN_REQ:
		{
			dna_info::LoginRequest message;
			if (message.ParseFromCodedStream(&payload_input_stream) == false)
			{
				std::cerr << "[Error] parse error" << std::endl;
				break;
			}
			handler.Handle(message, p_session_id);
		}
		break;
		/*
		case dna_info::LOGIN_RES:
		{
			dna_info::LoginResponse message;
			if (message.ParseFromCodedStream(&payload_input_stream) == false)
			{
				std::cerr << "[Error] parse error" << std::endl;
				break;
			}
			handler.Handle(message);
		}
		break;
		case dna_info::USER_INFO:
		{
			dna_info::UserInfo message;
			if (message.ParseFromCodedStream(&payload_input_stream) == false)
			{
				std::cerr << "[Error] parse error" << std::endl;
				break;
			}
			handler.Handle(message);
		}
		break;
		*/
		case dna_info::SYNC_INFO_C:
		{
			dna_info::SyncInfo_C message;
			if (message.ParseFromCodedStream(&payload_input_stream) == false)
			{
				std::cerr << "[Error] parse error" << std::endl;
				break;
			}
			handler.Handle(message, p_session_id);
		}
		break;
		/*
		case dna_info::SYNC_INFO_S:
		{
			dna_info::SyncInfo_S message;
			if (message.ParseFromCodedStream(&payload_input_stream) == false)
			{
				std::cerr << "[Error] parse error" << std::endl;
				break;
			}
			handler.Handle(message);
		}
		break;
		*/
		case dna_info::CHAT_REQ:
		{
		}
		break;
		/*
		case dna_info::CHAT_RES:
		{
		}
		break;
		*/
		}
	}

	//return remain_size;
	return nRead;
}

void WriteMessageToStream(
	const protobuf::Message& message,
	dna_info::packet_type message_type,
	protobuf::io::CodedOutputStream& stream)
{
	PacketHeader header;
	header.size = message.ByteSize();
	header.type = message_type;
	stream.WriteRaw(&header, sizeof(PacketHeader));
	message.SerializeToCodedStream(&stream);
}

/*
protobuf::uint8* MakePacket(
	int p_size, boost::function<void (
		const protobuf::Message&,
		dna_info::packet_type,
		protobuf::io::CodedOutputStream&)> handler)
{
	protobuf::uint8 *buf = new protobuf::uint8[p_size];
}
*/

/* 로그인 request 패킷을 처리하는 핸들러 */
void PacketHandler::Handle(const dna_info::LoginRequest& message, int p_session_id) const
{
	dna_info::LoginResponse res;
	protobuf::uint8 *packet;
	size_t size = 0;

	PrintMessage(message);

	/* 로그인 유효성 인증 루틴 */
	if ((m_server->login_server).Is_valid(message.id().c_str()))
		res.set_response_code(LOGIN_SUCCESFUL);
	else
		res.set_response_code(LOGIN_FAILED);
	
	/* 패킷 생성 */
	size = sizeof(PacketHeader) + res.ByteSize();
	packet = new protobuf::uint8[size];

	protobuf::io::ArrayOutputStream output_array_stream(packet, size);
	protobuf::io::CodedOutputStream output_coded_stream(&output_array_stream);

	WriteMessageToStream(res, dna_info::LOGIN_RES, output_coded_stream);

	/* 유저에게 로그인 response 전송 */
	m_server->GetSession(p_session_id)->Send_packet(false, packet, size);

	/* 유저가 로그인에 성공했다면 유저를 적절한 채널에 할당하고 채널 정보를 전송해줍니다. */
	if (res.response_code() == LOGIN_SUCCESFUL)
	{
		int channel_num;
		size_t _size = 0;
		protobuf::uint8 *_packet;
		dna_info::UserInfo user_info;

		/* 적절한 채널을 할당 받습니다. */
		channel_num = m_server->channel_manager.Get_BalancedArrangeNum();

		/* 세션에 유저 정보를 할당합니다. */
		User_info *user_info_s = new User_info;
		user_info_s->xpos = 0;
		user_info_s->ypos = 0;
		user_info_s->A_type_pow = 0;
		user_info_s->B_type_pow = 0;
		user_info_s->C_type_pow = 0;
		user_info_s->type = MALE;
		user_info_s->user_id = p_session_id;
		user_info_s->user_name = message.id();
		user_info_s->vec = 0;
		user_info_s->user_socket = &(m_server->GetSession(p_session_id)->Get_socket());
		m_server->GetSession(p_session_id)->Set_User(user_info_s);

		/* 세션에 채널을 설정합니다. */
		m_server->GetSession(p_session_id)->Set_Channel(channel_num);

		/* 채널에 유저를 바인딩합니다. */
		m_server->BindUser(user_info_s, channel_num);

		/* 유저에게 보낼 UserInfo 메시지를 설정합니다. */
		user_info.set_channel_num(channel_num);
		user_info.set_session_num(p_session_id);
		user_info.set_identify_id(message.id());

		_size += sizeof(PacketHeader) + user_info.ByteSize();
		_packet = new protobuf::uint8[_size];

		protobuf::io::ArrayOutputStream _output_array_stream(_packet, _size);
		protobuf::io::CodedOutputStream _output_coded_stream(&_output_array_stream);

		WriteMessageToStream(user_info, dna_info::USER_INFO, _output_coded_stream);

		/* 유저에게 UserInfo를 전송합니다. */
		m_server->GetSession(p_session_id)->Send_packet(false, _packet, _size);

		/* 해당 세션을 로그인 되어있는 세션으로 설정합니다. */
		m_server->GetSession(p_session_id)->Set_Login();
	}
}

/* 유저의 주기적인 sync 정보를 받아 처리하는 핸들러 */
void PacketHandler::Handle(const dna_info::SyncInfo_C& message, int p_session_id) const
{
	std::cout << "클라인트와 동기에 성공했습니다." << std::endl;

	/* 받아온 정보로 유저의 정보를 업데이트 합니다. */
	m_server->GetSession(p_session_id)->Update_User(message);
}