#pragma once

#include <string>
#include <deque>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "protocol.pb.h"
#include "PB_handler.h"
#include "Settings.h"

class GameServer;

class Session
{
public:
	Session(int Session_id, boost::asio::io_service& io_service, GameServer *server);
	~Session();

	void Init();

	void Send_packet(const bool b_Immediately, unsigned char *packet, size_t size);

	void Receive_packet();

	boost::asio::ip::tcp::socket& Get_socket() { return m_socket; }

	bool Is_active() const { return m_socket.is_open(); }
	int Get_session_id() { return session_id; }
	void Set_name(const std::string name) { user_name = name; }
	std::string Get_name() { return user_name; }
	
private:
	void handle_write(const boost::system::error_code& error,
		size_t bytes_transferred);

	void handle_receive(const boost::system::error_code& error,
		size_t bytes_transferred);


	boost::asio::ip::tcp::socket m_socket; // 클라이언트와 통신하는 소켓
	std::array<google::protobuf::uint8, MAX_RECEIVE_BUF_LEN> m_ReceiveBuffer; // 클라이언트에서 보낸 패킷을 받는 버퍼

	int m_nPacketBufferMark; // 현재 버퍼에 남아 있는 패킷의 크기
	google::protobuf::uint8 m_PacketBuffer[MAX_RECEIVE_BUF_LEN * 10]; // 패킷을 담아두는 버퍼

	std::deque<unsigned char *> send_queue; // 전송할 패킷들을 담아둘 큐(header + payload)

	int session_id; // 세션의 id
	bool isLogined;	// 현재 세션이 로그인 되어 있는 세션인가를 체크함
	int channel_num; // 현재 세션이 묶여있는 채널의 번호

	std::string user_name; // 유저의 이름

	int x_pos; // 유저의 x축 좌표
	int y_pos; // 유저의 y축 좌표
	int vec; // 유저의 마지막 방향
	int type; // 유저 캐릭터의 성별
	int A_type_pow; // 유저의 A 타입 DNA 수치
	int B_type_pow;
	int C_type_pow;

	GameServer *game_server;  // 현재 세션이 연결 되어있는 게임 서버
};