#pragma once

#include <iostream>

#include "Channel.h"
#include "Game_server.h"

Channel::Channel(int p_channel_num, GameServer *server)
	: m_Channel_num(p_channel_num), m_game_server(server)
{
}

Channel::~Channel()
{
	std::cout << "[channel " << m_Channel_num << "] 채널을 종료합니다." << std::endl;

	/* 유저가 한명이라도 존재하는 경우 */
	if (!m_UserInfo.empty())
	{
		m_game_server->CloseSession(m_UserInfo.begin()->second->user_id); // 유저의 세션을 종료합니다.
		m_UserInfo.erase(m_UserInfo.begin()); // 유저 리스트에서 해당 유저를 삭제합니다.
	}
}

void Channel::Init()
{
	m_nUserNum = 0; // 유저 수를 0으로 세팅합니다.
}

void Channel::Start()
{
	std::cout << "[channel " << m_Channel_num << "] 채널을 시작합니다." << std::endl;
}

bool Channel::Is_UniqueName(std::string user_name) const
{
	/* 채널내에 요청한 user_name이 이미 존재하는지를 체크합니다. */
	if (m_UserInfo.count(user_name) > 0)
		return false;
	else
		return true;
}

void Channel::Set_user(User_info *user)
{
	/* 새로운 유저에 대한 정보를 맵에 저장한다. */
	m_UserInfo[user->user_name] = user;
	m_nUserNum++;
}

void Channel::Delete_user(std::string user_name)
{
	/* 해당 유저의 정보를 삭제하고 자원을 반환한다. */
	delete[] m_UserInfo[user_name];
	m_UserInfo.erase(user_name);
	m_nUserNum--;
}

int Channel::Get_user_num() const
{
	return m_nUserNum;
}

ChannelBalancer::ChannelBalancer(GameServer *p_game_server)
	: m_game_server(p_game_server), m_ChannelNum(0), m_Channel_count(0)
{
}

ChannelBalancer::~ChannelBalancer()
{
	std::cout << "채널 manager를 종료합니다." << std::endl;

	/* 모든 채널을 종료합니다. */
	while (!m_Channel_list.empty())
	{
		delete[](m_Channel_list.begin()->second);
		m_Channel_list.erase(m_Channel_list.begin());
	}
}

void ChannelBalancer::Start()
{
	std::cout << "채널 load manager를 시작합니다." << std::endl;
}

void ChannelBalancer::Init()
{
	for (int i = 0; i < MIN_CHANNEL_NUM; i++)
	{
		Channel *channel = new Channel(i, m_game_server);
		
		channel->Start();
		channel->Init();
		m_Channel_list[i] = channel;

		m_ChannelNum++;
		m_Channel_count++;
	}
}

int ChannelBalancer::Get_BalancedArrangeNum()
{
	std::map<int, Channel *>::iterator suitable_channel;

	/* 전체 채널을 순회해서 가장 유저 수가 적은 채널을 찾습니다. */
	suitable_channel = m_Channel_list.begin();
	for (std::map<int, Channel *>::iterator it = m_Channel_list.begin();
		it != m_Channel_list.end(); ++it)
	{
		int current_user_num = it->second->Get_user_num();

		if (current_user_num < suitable_channel->second->Get_user_num())
			suitable_channel = it;
	}

	return suitable_channel->first;
}

bool ChannelBalancer::Is_UniqueName(std::string user_name) 
{
	bool is_valid = true;

	/* 해당 채널에서 user_name이 있는지 체크합니다. */
	for (std::map<int, Channel *>::iterator it = m_Channel_list.begin();
		it != m_Channel_list.end();
		++it)
	{
		if (!it->second->Is_UniqueName(user_name))
		{
			is_valid = false;
			break;
		}
	}

	if (is_valid == true)
		return true;
	else
		return false;
}

void ChannelBalancer::Optimize_Channel()
{

}

void ChannelBalancer::bind_User(User_info *p_user_info, int p_channel_num)
{
	m_Channel_list[p_channel_num]->Set_user(p_user_info);
}

void ChannelBalancer::Create_Channel()
{
	Channel *new_channel = new Channel(m_Channel_count, m_game_server);
	m_Channel_list[m_Channel_count] = new_channel;
	m_ChannelNum++;
	m_Channel_count++;
}

void ChannelBalancer::Delete_Channel(int channel_num)
{
	delete[] m_Channel_list[channel_num];
	m_Channel_list.erase(channel_num);
	m_ChannelNum--;
}