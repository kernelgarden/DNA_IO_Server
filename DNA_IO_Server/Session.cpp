#include <iostream>

#include "Session.h"
#include "Game_server.h"

/*
* 세션에 대한 초기화를 수행한다.
*/
Session::Session(int Session_id, boost::asio::io_service& io_service, GameServer *server)
	: session_id(Session_id), m_socket(io_service), game_server(server)
{
}

/*
* 세션을 정리하고 자원들을 반환한다.
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
* 세션을 초기화한다.
*/
void Session::Init()
{
	m_nPacketBufferMark = 0;
	// 유저 정보 초기화 필요
}

/*
* 해당 세션에 유저 정보를 세팅한다.
*/
void Session::Set_User(User_info *p_user_info)
{
	info = p_user_info;
}

/*
* 해당 세션에 채널을 저장합니다.
*/
void Session::Set_Channel(int p_channel_num)
{
	channel_num = p_channel_num;
}

/*
* 세션에 연결된 클라이언트로 패킷을 전송한다.
*/
void Session::Send_packet(const bool b_Immediately, unsigned char *packet, size_t size)
{
	unsigned char *SendData = nullptr;

	/* 데이터를 다 전송할 때까지 버퍼에 담아둔다. */

	/* 데이터큐가 쌓여 있는경우 */
	if (b_Immediately == false)
	{
		SendData = new unsigned char[size];
		memcpy(SendData, packet, size);
		delete[] packet;

		send_queue.push_back(SendData);
		send_queue_size.push_back(size);
	}
	else  /* handle write에서 send queue를 비우기 위해 호출한 경우 */
	{
		SendData = send_queue.front();
		size = send_queue_size.front();
	}

	/* send queue가 쌓여있는 경우 */
	if (b_Immediately == false && send_queue.size() > 1)
		return;

	/* 소켓에 비동기 쓰기를 수행한다. */
	boost::asio::async_write(m_socket, boost::asio::buffer(SendData, size),
		boost::bind(&Session::handle_write, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
	);
}

/* 
* 데이터 receive를 시작한다.
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

	/* queue에 보내야하는 패킷이 남아있다면 queue를 비우는 작업을 수행한다. */
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

		game_server->CloseSession(session_id);  // 에러가 발생하여 세션과 연결을 종료합니다.
	}
	else
	{
		// receive 버퍼에 저장된 패킷을 packet 버퍼로 복사합니다.
		memcpy(&m_PacketBuffer[m_nPacketBufferMark], m_ReceiveBuffer.data(), bytes_transferred);

		int nPacketData = m_nPacketBufferMark + bytes_transferred;  // 기존에 있던 데이터 + 새로 받아온 양
		int nReadData = 0;
		
		protobuf::io::ArrayInputStream input_array_stream(&m_PacketBuffer, nPacketData);
		protobuf::io::CodedInputStream input_coded_stream(&input_array_stream);

		nReadData = Process_packet(input_coded_stream,
			game_server->packet_handler, session_id); // 세션의 패킷 버퍼에 쌓인 패킷들을 처리하고 읽어들인 패킷 데이터의 양을 저장합니다.
		m_nPacketBufferMark = nPacketData - nReadData; // 남은 데이터의 양
		//m_nPacketBufferMark = Process_packet(input_coded_stream, 
		//	game_server->packet_handler, session_id); // 세션의 패킷 버퍼에 쌓인 패킷들을 처리하고 남은 패킷 데이터의 양을 저장합니다.
		//nReadData = nPacketData - m_nPacketBufferMark;  // 처리한 데이터의 양

		if (m_nPacketBufferMark > 0) // 남는 데이터를 저장합니다.
		{
			google::protobuf::uint8 TempBuffer[MAX_RECEIVE_BUF_LEN] = { 0, };
			memcpy(&TempBuffer[0], &m_PacketBuffer[nReadData], m_nPacketBufferMark); // 마지막으로 패킷을 읽었던 곳부터 복사를 함
			memcpy(&m_PacketBuffer[0], &TempBuffer[0], m_nPacketBufferMark);
		}

		Receive_packet();
	}
}