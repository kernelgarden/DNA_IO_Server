#pragma once

#include <deque>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "protocol.pb.h"

class LoginServer
{
public:
	LoginServer();
	LoginServer(boost::asio::io_service& io_service);
	~LoginServer();

	void Init();
	
	void Start();

private:
	void PostResponse();

	void ReceiveRequest();

	void handle_post(const boost::system::error_code& error,
		size_t byte_transfferd);

	void handle_receive(const boost::system::error_code& error,
		size_t byte_transfferd);
};