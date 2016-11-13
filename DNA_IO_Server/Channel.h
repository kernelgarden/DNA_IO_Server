#pragma once

#include <map>
#include <vector>
#include <string>

#include "User_info.h"

/*
* ���ǵ��� ������ ä�� ��ü
*/
class Channel
{
public:
	Channel();
	~Channel();

	void Init(); // �ʻ��� �ڿ����� �����մϴ�.

	void Start();

	bool Is_UniqueName(std::string) const; // ���� ä�� ������ �ش� �̸��� ���ϼ��� üũ�ϴ� ��ƾ

	void Set_user(User_info *user); // �ش� ������ ���� ä�ο� ���ε���Ű�� ��ƾ

	void Delete_user(std::string); // �ش� �̸��� ���� ������ ����� ��ƾ

private:

	int m_Channel_num; // ä���� ��ȣ

	int m_nUserNum; // �ش� ä�ο� �ִ� ������ ��
	std::map<std::string, User_info> UserInfo; // ������ �̸��� Ű�� ������ ������ �����ϴ� ��
};

/*
* ä�ε��� ȿ�������� �����ϴ� ä�� �Ŵ���
*/
class ChannelBalancer
{
public:
	ChannelBalancer();
	~ChannelBalancer();

	void Start();

	void Init();

	int Get_BalancedArrangeNum(); // ���� ������ ä���� ��ȣ�� �������ִ� ��ƾ

	bool Is_UniqueName(std::string) const; // �ش� �̸��� ���ϼ��� üũ�ϴ� ��ƾ

	void Optimize_Channel(); // �����ð��� �ֱ�� ������ ����ִ� ä���� üũ�ؼ� �����ϴ� ��ƾ
private:

	std::vector<Channel> m_Channel_list; // ä���� ����Ʈ

	int m_ChannelNum; // ���� �����ϴ� ä���� ��
};