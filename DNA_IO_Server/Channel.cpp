#pragma once

#include "Channel.h"

Channel::Channel()
{

}

Channel::~Channel()
{

}

void Channel::Init()
{

}

void Channel::Start()
{

}

bool Channel::Is_UniqueName(std::string) const
{
	return false;
}

void Channel::Set_user(User_info *user)
{

}

void Delete_user(std::string)
{

}

ChannelBalancer::ChannelBalancer()
{

}

ChannelBalancer::~ChannelBalancer()
{

}

void ChannelBalancer::Start()
{

}

void ChannelBalancer::Init()
{

}

int ChannelBalancer::Get_BalancedArrangeNum()
{
	return 0;
}

bool ChannelBalancer::Is_UniqueName(std::string) const
{
	return false;
}

void ChannelBalancer::Optimize_Channel()
{

}