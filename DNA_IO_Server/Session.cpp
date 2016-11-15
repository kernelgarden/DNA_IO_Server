#include <iostream>

#include "Session.h"
#include "Game_server.h"

/*
* ���ǿ� ���� �ʱ�ȭ�� �����Ѵ�.
*/
Session::Session(int Session_id, boost::asio::io_service& io_service, GameServer *server)
	: session_id(Session_id), m_socket(io_service), game_server(server)
{
}

/*
* ������ �����ϰ� �ڿ����� ��ȯ�Ѵ�.
*/
Session::~Session()
{
	while (send_queue.empty() == false)
	{
		delete[] send_queue.front();
		send_queue.pop_front();
		send_queue_size.pop_front();
	}
}

/*
* ������ �ʱ�ȭ�Ѵ�.
*/
void Session::Init()
{
	m_nPacketBufferMark = 0;
	// ���� ���� �ʱ�ȭ �ʿ�
}

/*
* �ش� ���ǿ� ���� ������ �����Ѵ�.
*/
void Session::Set_User(User_info *p_user_info)
{
	info = p_user_info;
}

/*
* �ش� ���ǿ� ä���� �����մϴ�.
*/
void Session::Set_Channel(int p_channel_num)
{
	channel_num = p_channel_num;
}

/*
* ���ǿ� ����� Ŭ���̾�Ʈ�� ��Ŷ�� �����Ѵ�.
*/
void Session::Send_packet(const bool b_Immediately, unsigned char *packet, size_t size)
{
	unsigned char *SendData = nullptr;

	/* �����͸� �� ������ ������ ���ۿ� ��Ƶд�. */

	/* ������ť�� �׿� �ִ°�� */
	if (b_Immediately == false)
	{
		SendData = new unsigned char[size];
		memcpy(SendData, packet, size);
		delete[] packet;

		send_queue.push_back(SendData);
		send_queue_size.push_back(size);
	}
	else  /* handle write���� send queue�� ���� ���� ȣ���� ��� */
	{
		SendData = send_queue.front();
		size = send_queue_size.front();
	}

	/* send queue�� �׿��ִ� ��� */
	if (b_Immediately == false && send_queue.size() > 1)
		return;

	/* ���Ͽ� �񵿱� ���⸦ �����Ѵ�. */
	boost::asio::async_write(m_socket, boost::asio::buffer(SendData, size),
		boost::bind(&Session::handle_write, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
	);
}

/* 
* ������ receive�� �����Ѵ�.
*/
void Session::Receive_packet()
{
	m_socket.async_read_some(
		boost::asio::buffer(m_ReceiveBuffer),
		boost::bind(&Session::handle_receive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
	);
}

void Session::handle_write(const boost::system::error_code& error,
	size_t bytes_transferred)
{
	delete[] send_queue.front();
	send_queue.pop_front();

	int size = send_queue_size.front();
	send_queue_size.pop_front();

	/* queue�� �������ϴ� ��Ŷ�� �����ִٸ� queue�� ���� �۾��� �����Ѵ�. */
	if (send_queue.empty() == false)
	{
		Send_packet(true, send_queue.front(), size);
	}
}

void Session::handle_receive(const boost::system::error_code& error,
	size_t bytes_transferred)
{
	if (error)
	{
		if (error == boost::asio::error::eof)
		{
			std::cerr << "[ERROR] Connection to the client has been lost." << std::endl;
		}
		else
		{
			std::cerr << "[ERROR] No: " << error.value() << " Message: "
				<< error.message() << std::endl;
		}

		game_server->CloseSession(session_id);  // ������ �߻��Ͽ� ���ǰ� ������ �����մϴ�.
	}
	else
	{
		// receive ���ۿ� ����� ��Ŷ�� packet ���۷� �����մϴ�.
		memcpy(&m_PacketBuffer[m_nPacketBufferMark], m_ReceiveBuffer.data(), bytes_transferred);

		int nPacketData = m_nPacketBufferMark + bytes_transferred;  // ������ �ִ� ������ + ���� �޾ƿ� ��
		int nReadData = 0;
		
		protobuf::io::ArrayInputStream input_array_stream(&m_PacketBuffer, nPacketData);
		protobuf::io::CodedInputStream input_coded_stream(&input_array_stream);

		nReadData = Process_packet(input_coded_stream,
			game_server->packet_handler, session_id); // ������ ��Ŷ ���ۿ� ���� ��Ŷ���� ó���ϰ� �о���� ��Ŷ �������� ���� �����մϴ�.
		m_nPacketBufferMark = nPacketData - nReadData; // ���� �������� ��
		//m_nPacketBufferMark = Process_packet(input_coded_stream, 
		//	game_server->packet_handler, session_id); // ������ ��Ŷ ���ۿ� ���� ��Ŷ���� ó���ϰ� ���� ��Ŷ �������� ���� �����մϴ�.
		//nReadData = nPacketData - m_nPacketBufferMark;  // ó���� �������� ��

		if (m_nPacketBufferMark > 0) // ���� �����͸� �����մϴ�.
		{
			google::protobuf::uint8 TempBuffer[MAX_RECEIVE_BUF_LEN] = { 0, };
			memcpy(&TempBuffer[0], &m_PacketBuffer[nReadData], m_nPacketBufferMark); // ���������� ��Ŷ�� �о��� ������ ���縦 ��
			memcpy(&m_PacketBuffer[0], &TempBuffer[0], m_nPacketBufferMark);
		}

		Receive_packet();
	}
}