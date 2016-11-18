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

void Channel::Sync_each_user()
{
	size_t size = 0;
	protobuf::uint8 *packet;
	dna_info::SyncInfo_S users_sync_info;

	/* 해당 채널의 유저들의 데이터들을 뽑아서 메세지화한다. */
	for (std::map<std::string, User_info *>::iterator it = m_UserInfo.begin();
		it != m_UserInfo.end(); ++it)
	{
		dna_info::SyncInfo_S_User *user_sync_info = users_sync_info.add_users();

		user_sync_info->set_user_id(it->second->user_name);
		user_sync_info->set_a_type_pow(it->second->A_type_pow);
		user_sync_info->set_b_type_pow(it->second->B_type_pow);
		user_sync_info->set_c_type_pow(it->second->C_type_pow);
		user_sync_info->set_x_pos(it->second->xpos);
		user_sync_info->set_y_pos(it->second->ypos);
		user_sync_info->set_vec(it->second->vec);
	}

	size += sizeof(PacketHeader) + users_sync_info.ByteSize();
	packet = new protobuf::uint8[size];

	protobuf::io::ArrayOutputStream output_array_stream(packet, size);
	protobuf::io::CodedOutputStream output_coded_stream(&output_array_stream);

	WriteMessageToStream(users_sync_info, dna_info::SYNC_INFO_S, output_coded_stream);

	/* 채널 내 모든 유저들에게 데이터를 전송한다. */
	for (std::map<std::string, User_info *>::iterator it = m_UserInfo.begin();
		it != m_UserInfo.end(); ++it)
	{
		m_game_server->GetSession(it->second->user_id)->Send_packet(false, packet, size);
	}
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

void ChannelBalancer::Sync_Channel(const boost::system::error_code& error,
	boost::asio::steady_timer *p_timer)
{
	for (std::map<int, Channel *>::iterator it = m_Channel_list.begin();
		it != m_Channel_list.end(); ++it)
	{
		it->second->Sync_each_user();
	}

	m_game_server->StartSync();
}