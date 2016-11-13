#pragma once

#include <iostream>
#include <deque>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "protocol.pb.h"

class ChannelBalancer;

class LoginServer
{
public:
	LoginServer();
	LoginServer(boost::asio::io_service& io_service);
	~LoginServer();

	void Init(ChannelBalancer* channel_manager);
	
	void Start();

	bool Is_valid(std::string p_id); // ������ ���̵� ��ȿ������ üũ�ϴ� ��ƾ

private:
	void SendResponse();

	void ReceiveRequest();

	void handle_post(const boost::system::error_code& error,
		size_t byte_transfferd);

	void handle_receive(const boost::system::error_code& error,
		size_t byte_transfferd);

	ChannelBalancer *m_channel_manager;
};