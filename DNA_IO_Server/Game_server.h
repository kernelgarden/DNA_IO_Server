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

		/* 열려있는 세션들을 닫고 자원을 반환합니다 */
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

		/* 채널 manager 서버를 시작시킵니다. */
		channel_manager.Init();
		channel_manager.Start();

		/* 로그인 로직을 처리할 서버를 시작시킵니다 */
		login_server.Init(&channel_manager);
		login_server.Start();

		/* 세션들을 초기화해둡니다. */
		for (int i = 0; i < MAX_SESSION_NUM; i++)
		{
			session_list.push_back(new Session(i, m_acceptor.get_io_service(), this)); 
			session_queue.push_back(i);
		}
	}

	/*
	* game server를 구동합니다.
	*/
	void Start()
	{
		std::cout << "game server is running..." << std::endl;

		PostAccept();
	}

	/*
	* sessionID에 해당하는 세션을 정리하고 자원을 해제합니다.
	*/
	void CloseSession(int sessionID)
	{
		std::cout << "[Close Session] session_id: " << sessionID << std::endl;

		session_list[sessionID]->Get_socket().close();
		session_queue.push_back(sessionID);

		if (m_bIsAccepting == false)  // 세션이 부족해 accept를 못하고 있는 경우 다시 accept를 수행합니다.
			PostAccept();
	}

	/*
	* 해당 sessionID에 해당하는 세션을 반환합니다.
	* 만약 그 세션이 활성화 되어있는 세션이 아니라면 nullptr을 리턴합니다.
	*/
	Session *GetSession(int sessionID)
	{
		if (session_list[sessionID]->Is_active())
			return session_list[sessionID];
		else
			return nullptr;
	}

	PacketHandler packet_handler; // 패킷을 처리하는 핸들러 클래스
	LoginServer login_server;  // 로그인 처리를 수행할 서버

private:
	/*
	* accept를 수행합니다.
	*/
	bool PostAccept()
	{
		if (session_queue.empty()) // 가용가능한 session이 없다면 accept를 중지합니다.
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
	* 연결된 세션에 대한 초기화를 수행하고 receive를 시작한다.
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

	ChannelBalancer channel_manager; // 채널들을 관리하는 매니지 서버

	std::vector<Session *> session_list;	// 세션 리스트
	std::deque<int> session_queue;	// 할당 가능한 세션 id를 담고있는 큐

	bool m_bIsAccepting; // 서버가 현재 accept를 수행하고 있는지에 대한 정보

	boost::asio::ip::tcp::acceptor m_acceptor; // 게임서버의 acceptor
};