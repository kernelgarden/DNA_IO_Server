#pragma once

#include <map>
#include <vector>
#include <string>
#include <boost/asio/steady_timer.hpp>

#include "User_info.h"
#include "Settings.h"

class GameServer;

/*
* 세션들이 접속할 채널 객체
*/
class Channel
{
public:
	Channel(int p_channel_num, GameServer *server);  // 인자로 채널의 번호를 할당받고 채널을 생성합니다.
	~Channel();

	void Init(); // 맵상의 자원들을 세팅합니다.

	void Start();

	bool Is_UniqueName(std::string user_name) const; // 현재 채널 내에서 해당 이름의 유일성을 체크하는 루틴

	void Set_user(User_info *user); // 해당 유저를 현재 채널에 바인딩시키는 루틴

	void Delete_user(std::string user_name); // 해당 이름을 가진 유저를 지우는 루틴

	int Get_user_num() const; // 채널에 있는 유저 수를 리턴하는 루틴

	void Sync_each_user(); // 해당 채널 내에 클라이언트들로 동기 정보를 전송하는 루틴

private:

	GameServer *m_game_server; // 게임 서버

	int m_Channel_num; // 채널의 번호

	int m_nUserNum; // 해당 채널에 있는 유저의 수
	std::map<std::string, User_info *> m_UserInfo; // 유저의 이름을 키로 유저의 정보를 관리하는 맵
};

/*
* 채널들을 효율적으로 관리하는 채널 매니저
*/
class ChannelBalancer
{
public:
	ChannelBalancer(GameServer *p_game_server);
	~ChannelBalancer();

	void Start();

	void Init(); // 채널들을 미리 준비해두고 유저를 받을 준비를 하는 루틴

	int Get_BalancedArrangeNum(); // 가장 최적의 채널의 번호를 리턴해주는 루틴

	bool Is_UniqueName(std::string user_name); // 해당 이름의 유일성을 체크하는 루틴

	void Optimize_Channel(); // 일정시간을 주기로 채널의 수를 효과적으로 관리하는 루틴

	void bind_User(User_info *p_user_info, int p_channel_num); // 해당 채널에 세션을 바인딩합니다.

	void Create_Channel(); // 새로운 채널을 생성하는 루틴

	void Delete_Channel(int channel_id); // 해당 채널 id를 가진 채널을 종료하는 루틴

	void Sync_Channel(const boost::system::error_code& error,
		boost::asio::steady_timer *p_timer); // 각 채널에 연결된 유저들에게 동기 정보를 전송합니다.
private:

	GameServer *m_game_server;  // 게임 서버

	std::map<int, Channel *> m_Channel_list; // 채널의 리스트

	int m_ChannelNum; // 현재 존재하는 채널의 수
	int m_Channel_count; // 지금 까지 생성된 채널 수
};