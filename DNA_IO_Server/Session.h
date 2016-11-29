#pragma once

#include <string>
#include <deque>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "protocol.pb.h"
#include "PB_handler.h"
#include "Settings.h"
#include "User_info.h"

class GameServer;

class Session
{
public:
	Session(int Session_id, boost::asio::io_service& io_service, GameServer *server);
	~Session();

	void Init();

	void Send_packet(const bool b_Immediately, unsigned char *packet, size_t size);

	void Send_packet(const bool b_Immediately, unsigned char *packet, size_t size, bool auto_deletion);

	void Receive_packet();

	void Clear_session();

	void Set_User(User_info *p_user_info);

	void Set_Channel(int p_channel_num);
	
	int Get_Channel() const { return channel_num; }
	
	bool Is_Logined() const { return isLogined; }

	void Set_Login() { isLogined = true; }

	void Update_User(const dna_info::SyncInfo_C& message);

	boost::asio::ip::tcp::socket& Get_socket() { return m_socket; }

	bool Is_active() const { return m_socket.is_open(); }

	int Get_session_id() { return session_id; }
	void Set_name(const std::string name) { info->user_name = name; }
	std::string Get_name() { return info->user_name; }
	
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
	std::deque<int> send_queue_size; // send_queue�� size�� ������ �ִ� ť

	int session_id; // ������ id
	bool isLogined;	// ���� ������ �α��� �Ǿ� �ִ� �����ΰ��� üũ��
	int channel_num; // ���� ������ �����ִ� ä���� ��ȣ

	/*
		int user_id; // ������ ���� id
		std::string user_name; // ������ �̸�
		int x_pos; // ������ x�� ��ǥ
		int y_pos; // ������ y�� ��ǥ
		int vec; // ������ ������ ����
		int type; // ���� ĳ������ ����
		int A_type_pow; // ������ A Ÿ�� DNA ��ġ
		int B_type_pow;
		int C_type_pow;
	*/
	User_info *info; // �������� ����

	GameServer *game_server;  // ���� ������ ���� �Ǿ��ִ� ���� ����
};