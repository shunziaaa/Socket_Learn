#pragma once
#include <string>
#include "cmd_id.h"
#include <iostream>
using std::cout;
using std::endl;
using std::string;
struct ProtoHead
{
	int cmd_id;
	int length;
};

// 客户端连接
struct SC_NewClient :public ProtoHead
{
	SC_NewClient()
	{
		cmd_id = cmd_sc_new_clinet;
		length = sizeof(SC_NewClient);
	}

	string ip;
};

// 客户端断开
struct SC_Disconnect :public ProtoHead
{
	SC_Disconnect()
	{
		cmd_id = cmd_sc_disconnect_clinet;
		length = sizeof(SC_Disconnect);
	}

	int sock;
};

struct CS_Login :public ProtoHead
{
	CS_Login()
	{
		cmd_id = cmd_cs_login;
		length = sizeof(CS_Login);
	}

	string name;
	string passward;
};

struct SC_Login :public ProtoHead
{
	SC_Login()
	{
		cmd_id = cmd_sc_login;
		length = sizeof(SC_Login);
	}

	int result;
};