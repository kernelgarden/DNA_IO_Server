#pragma once

#include <string>
#include <boost/asio.hpp>
/*
* 유저 정보를 관리하는 객체
*/
struct User_info
{
	int user_id;
	std::string user_name;
	int xpos;
	int ypos;
	int vec;
	int type;
	int A_type_pow;
	int B_type_pow;
	int C_type_pow;
	boost::asio::ip::tcp::socket *user_socket;
};