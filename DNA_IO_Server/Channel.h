#pragma once

#include <map>
#include <vector>
#include <string>

#include "User_info.h"

/*
* 세션들이 접속할 채널 객체
*/
class Channel
{
public:
	Channel();
	~Channel();

	void Init(); // 맵상의 자원들을 세팅합니다.

	void Start();

	bool Is_UniqueName(std::string) const; // 현재 채널 내에서 해당 이름의 유일성을 체크하는 루틴

	void Set_user(User_info *user); // 해당 유저를 현재 채널에 바인딩시키는 루틴

	void Delete_user(std::string); // 해당 이름을 가진 유저를 지우는 루틴

private:

	int m_Channel_num; // 채널의 번호

	int m_nUserNum; // 해당 채널에 있는 유저의 수
	std::map<std::string, User_info> UserInfo; // 유저의 이름을 키로 유저의 정보를 관리하는 맵
};

/*
* 채널들을 효율적으로 관리하는 채널 매니저
*/
class ChannelBalancer
{
public:
	ChannelBalancer();
	~ChannelBalancer();

	void Start();

	void Init();

	int Get_BalancedArrangeNum(); // 가장 최적의 채널의 번호를 리턴해주는 루틴

	bool Is_UniqueName(std::string) const; // 해당 이름의 유일성을 체크하는 루틴

	void Optimize_Channel(); // 일정시간을 주기로 유저가 비어있는 채널을 체크해서 관리하는 루틴
private:

	std::vector<Channel> m_Channel_list; // 채널의 리스트

	int m_ChannelNum; // 현재 존재하는 채널의 수
};