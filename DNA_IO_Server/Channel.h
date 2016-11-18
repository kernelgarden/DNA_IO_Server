#pragma once

#include <map>
#include <vector>
#include <string>
#include <boost/asio/steady_timer.hpp>

#include "User_info.h"
#include "Settings.h"

class GameServer;

/*
* ���ǵ��� ������ ä�� ��ü
*/
class Channel
{
public:
	Channel(int p_channel_num, GameServer *server);  // ���ڷ� ä���� ��ȣ�� �Ҵ�ް� ä���� �����մϴ�.
	~Channel();

	void Init(); // �ʻ��� �ڿ����� �����մϴ�.

	void Start();

	bool Is_UniqueName(std::string user_name) const; // ���� ä�� ������ �ش� �̸��� ���ϼ��� üũ�ϴ� ��ƾ

	void Set_user(User_info *user); // �ش� ������ ���� ä�ο� ���ε���Ű�� ��ƾ

	void Delete_user(std::string user_name); // �ش� �̸��� ���� ������ ����� ��ƾ

	int Get_user_num() const; // ä�ο� �ִ� ���� ���� �����ϴ� ��ƾ

	void Sync_each_user(); // �ش� ä�� ���� Ŭ���̾�Ʈ��� ���� ������ �����ϴ� ��ƾ

private:

	GameServer *m_game_server; // ���� ����

	int m_Channel_num; // ä���� ��ȣ

	int m_nUserNum; // �ش� ä�ο� �ִ� ������ ��
	std::map<std::string, User_info *> m_UserInfo; // ������ �̸��� Ű�� ������ ������ �����ϴ� ��
};

/*
* ä�ε��� ȿ�������� �����ϴ� ä�� �Ŵ���
*/
class ChannelBalancer
{
public:
	ChannelBalancer(GameServer *p_game_server);
	~ChannelBalancer();

	void Start();

	void Init(); // ä�ε��� �̸� �غ��صΰ� ������ ���� �غ� �ϴ� ��ƾ

	int Get_BalancedArrangeNum(); // ���� ������ ä���� ��ȣ�� �������ִ� ��ƾ

	bool Is_UniqueName(std::string user_name); // �ش� �̸��� ���ϼ��� üũ�ϴ� ��ƾ

	void Optimize_Channel(); // �����ð��� �ֱ�� ä���� ���� ȿ�������� �����ϴ� ��ƾ

	void bind_User(User_info *p_user_info, int p_channel_num); // �ش� ä�ο� ������ ���ε��մϴ�.

	void Create_Channel(); // ���ο� ä���� �����ϴ� ��ƾ

	void Delete_Channel(int channel_id); // �ش� ä�� id�� ���� ä���� �����ϴ� ��ƾ

	void Sync_Channel(const boost::system::error_code& error,
		boost::asio::steady_timer *p_timer); // �� ä�ο� ����� �����鿡�� ���� ������ �����մϴ�.
private:

	GameServer *m_game_server;  // ���� ����

	std::map<int, Channel *> m_Channel_list; // ä���� ����Ʈ

	int m_ChannelNum; // ���� �����ϴ� ä���� ��
	int m_Channel_count; // ���� ���� ������ ä�� ��
};