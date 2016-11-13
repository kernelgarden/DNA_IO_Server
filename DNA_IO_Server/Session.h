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


	boost::asio::ip::tcp::socket m_socket; // Ŭ���̾�Ʈ�� ����ϴ� ����
	std::array<google::protobuf::uint8, MAX_RECEIVE_BUF_LEN> m_ReceiveBuffer; // Ŭ���̾�Ʈ���� ���� ��Ŷ�� �޴� ����

	int m_nPacketBufferMark; // ���� ���ۿ� ���� �ִ� ��Ŷ�� ũ��
	google::protobuf::uint8 m_PacketBuffer[MAX_RECEIVE_BUF_LEN * 10]; // ��Ŷ�� ��Ƶδ� ����

	std::deque<unsigned char *> send_queue; // ������ ��Ŷ���� ��Ƶ� ť(header + payload)

	int session_id; // ������ id
	bool isLogined;	// ���� ������ �α��� �Ǿ� �ִ� �����ΰ��� üũ��
	int channel_num; // ���� ������ �����ִ� ä���� ��ȣ

	std::string user_name; // ������ �̸�

	int x_pos; // ������ x�� ��ǥ
	int y_pos; // ������ y�� ��ǥ
	int vec; // ������ ������ ����
	int type; // ���� ĳ������ ����
	int A_type_pow; // ������ A Ÿ�� DNA ��ġ
	int B_type_pow;
	int C_type_pow;

	GameServer *game_server;  // ���� ������ ���� �Ǿ��ִ� ���� ����
};