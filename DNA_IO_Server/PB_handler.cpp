#include "PB_handler.h"

int Process_packet(protobuf::io::CodedInputStream& input_stream, const PacketHandler& handler)
{
	int remain_size;
	PacketHeader packet_header;

	// ��Ʈ�����κ��� ����� �о�ɴϴ�.
	while (input_stream.ReadRaw(&packet_header, sizeof(PacketHeader)))
	{
		// ��Ʈ���� ���� ������ �� �� �ִ� ���� �����Ϳ� ���� ���̸� �˾ƿɴϴ�.
		const void *payload_ptr = NULL;
		remain_size = 0;
		input_stream.GetDirectBufferPointer(&payload_ptr, &remain_size);
		if (remain_size < (signed)packet_header.size)
			break;

		// ��Ŷ�� �о�� ���� ��Ʈ���� �����մϴ�.
		protobuf::io::ArrayInputStream payload_array_stream(payload_ptr, packet_header.size);
		protobuf::io::CodedInputStream payload_input_stream(&payload_array_stream);

		// ��Ŷ ��ü��ŭ �����͸� �̵��մϴ�.
		input_stream.Skip(packet_header.size);

		// ��Ŷ�� �м��ؼ� �޽��� ������ ���� ó���� �մϴ�.
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
			handler.Handle(message);
		}
		break;
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
		case dna_info::SYNC_INFO_C:
		{
			dna_info::SyncInfo_C message;
			if (message.ParseFromCodedStream(&payload_input_stream) == false)
			{
				std::cerr << "[Error] parse error" << std::endl;
				break;
			}
			handler.Handle(message);
		}
		break;
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
		case dna_info::CHAT_REQ:
		{
		}
		break;
		case dna_info::CHAT_RES:
		{
		}
		break;
		}
	}

	return remain_size;
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