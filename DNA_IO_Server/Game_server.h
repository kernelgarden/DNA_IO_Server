#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <vector>

#include "protocol.pb.h"
#include "Login_server.h"
#include "Channel.h"
#include "Session.h"
#include "Settings.h"

class GameServer
{
public:
	GameServer(boost::asio::io_service& io_service)
		: packet_handler(this),
		m_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUM))
	{
		m_bIsAccepting = false;
	}

	~GameServer()
	{
		std::cout << "[Delete Session] Sessions are being deleted" << std::endl;

		/* �����ִ� ���ǵ��� �ݰ� �ڿ��� ��ȯ�մϴ� */
		for (size_t i = 0; i < MAX_SESSION_NUM; i++)
		{
			if (session_list[i]->Get_socket().is_open())
				session_list[i]->Get_socket().close();
			
			delete session_list[i];
		}
	}

	void Init()
	{
		std::cout << "[Make Session] Sessions are being created" << std::endl;

		/* ä�� manager ������ ���۽�ŵ�ϴ�. */
		channel_manager.Init();
		channel_manager.Start();

		/* �α��� ������ ó���� ������ ���۽�ŵ�ϴ� */
		login_server.Init(&channel_manager);
		login_server.Start();

		/* ���ǵ��� �ʱ�ȭ�صӴϴ�. */
		for (int i = 0; i < MAX_SESSION_NUM; i++)
		{
			session_list.push_back(new Session(i, m_acceptor.get_io_service(), this)); 
			session_queue.push_back(i);
		}
	}

	/*
	* game server�� �����մϴ�.
	*/
	void Start()
	{
		std::cout << "game server is running..." << std::endl;

		PostAccept();
	}

	/*
	* sessionID�� �ش��ϴ� ������ �����ϰ� �ڿ��� �����մϴ�.
	*/
	void CloseSession(int sessionID)
	{
		std::cout << "[Close Session] session_id: " << sessionID << std::endl;

		session_list[sessionID]->Get_socket().close();
		session_queue.push_back(sessionID);

		if (m_bIsAccepting == false)  // ������ ������ accept�� ���ϰ� �ִ� ��� �ٽ� accept�� �����մϴ�.
			PostAccept();
	}

	/*
	* �ش� sessionID�� �ش��ϴ� ������ ��ȯ�մϴ�.
	* ���� �� ������ Ȱ��ȭ �Ǿ��ִ� ������ �ƴ϶�� nullptr�� �����մϴ�.
	*/
	Session *GetSession(int sessionID)
	{
		if (session_list[sessionID]->Is_active())
			return session_list[sessionID];
		else
			return nullptr;
	}

	PacketHandler packet_handler; // ��Ŷ�� ó���ϴ� �ڵ鷯 Ŭ����
	LoginServer login_server;  // �α��� ó���� ������ ����

private:
	/*
	* accept�� �����մϴ�.
	*/
	bool PostAccept()
	{
		if (session_queue.empty()) // ���밡���� session�� ���ٸ� accept�� �����մϴ�.
		{
			m_bIsAccepting = false;
			return false;
		}

		m_bIsAccepting = true;
		
		int nSessionID = session_queue.front();
		session_queue.pop_front();

		m_acceptor.async_accept(session_list[nSessionID]->Get_socket(),
			boost::bind(&GameServer::handle_accept, this,
				nSessionID, boost::asio::placeholders::error)
		);

		return true;
	}

	/*
	* ����� ���ǿ� ���� �ʱ�ȭ�� �����ϰ� receive�� �����Ѵ�.
	*/
	void handle_accept(int nSessionID, const boost::system::error_code& error)
	{
		if (!error)
		{
			Session *session = session_list[nSessionID];

			session->Init();
			session->Receive_packet();

			PostAccept();
		}
		else
		{
			std::cerr << "[ERROR] No: " << error.value() << " Message: "
				<< error.message() << std::endl;
		}
	}

	ChannelBalancer channel_manager; // ä�ε��� �����ϴ� �Ŵ��� ����

	std::vector<Session *> session_list;	// ���� ����Ʈ
	std::deque<int> session_queue;	// �Ҵ� ������ ���� id�� ����ִ� ť

	bool m_bIsAccepting; // ������ ���� accept�� �����ϰ� �ִ����� ���� ����

	boost::asio::ip::tcp::acceptor m_acceptor; // ���Ӽ����� acceptor
};