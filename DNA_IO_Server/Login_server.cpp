#include "Login_server.h"
#include "Channel.h"

LoginServer::LoginServer()
{

}

LoginServer::LoginServer(boost::asio::io_service& io_service)
{

}

LoginServer::~LoginServer()
{

}

void LoginServer::Init(ChannelBalancer* channel_manager)
{
	m_channel_manager = channel_manager;
}

void LoginServer::Start()
{
	std::cout << "[Login Server] login server is running..." << std::endl;
}

void LoginServer::SendResponse()
{

}

void LoginServer::ReceiveRequest()
{

}

bool LoginServer::Is_valid(const std::string p_id)
{
	/* ���ӳ��� p_id�� ���� �̸��� ���� ������ �ִ��� üũ�մϴ�. */
	if (m_channel_manager->Is_UniqueName(p_id))
		return true;
	else
		return false;
}

void LoginServer::handle_post(const boost::system::error_code& error,
	size_t byte_transferred)
{

}

void LoginServer::handle_receive(const boost::system::error_code& error,
	size_t byte_transferred)
{

}