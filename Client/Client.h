#include <WinSock2.h>
#include <unordered_set>
#include <string>
#include <iostream>

#include "proto.h"

using std::unordered_set;
using std::string;

class Client
{
public:
	Client() 
	{
		m_sock = INVALID_SOCKET;
		m_isRun = false;
	}
	virtual ~Client()
	{
		Close();
	}

	bool Init()
	{
		// 已经开启环境
		if (m_sock != INVALID_SOCKET)
		{
			return true;
		}
		// 开启环境
		WORD w = MAKEWORD(2, 2);
		WSAData data;
		int ret = WSAStartup(w, &data);
		if (ret == INVALID_SOCKET)
		{
			ErrorLog("客户端环境开启失败");
			return false;
		}
		InfoLog("客户端环境开启");

		m_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (m_sock == INVALID_SOCKET)
		{
			ErrorLog("socket error");
			return false;
		}
	
		return true;
	}

	bool Connect(string ip, int port)
	{
		if (m_sock == INVALID_SOCKET && !Init())
		{
			return false;
		}

		sockaddr_in ser_addr = {};
		ser_addr.sin_family = AF_INET;
		ser_addr.sin_port = htons((short)port);
		ser_addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());

		int ret = connect(m_sock, (sockaddr*)&ser_addr, sizeof(sockaddr_in));
		if (ret == SOCKET_ERROR)
		{
			return false;
		}

		InfoLog("连接服务器...");
		m_isRun = true;
		return true;
	}

	void Close()
	{
		if (m_sock == INVALID_SOCKET)
		{
			return;
		}

		closesocket(m_sock);
		m_isRun = false;
	}

	bool IsRun()
	{
		return m_isRun;
	}

	void Run()
	{
		if (! IsRun())
		{
			return;
		}

		fd_set read_fd = {};
		fd_set write_fd = {};
		fd_set ex_fd = {};

		FD_ZERO(&read_fd);
		FD_ZERO(&write_fd);
		FD_ZERO(&ex_fd);
		FD_SET(m_sock, &read_fd);

		timeval t = { 1, 0 };
		select((int)m_sock, &read_fd, &write_fd, &ex_fd, &t);

		if (FD_ISSET(m_sock, &read_fd))
		{
			RecvMessage();
		}
	}

	void RecvMessage()
	{
		char buf[1024];
		int len = recv(m_sock, buf, sizeof(ProtoHead), 0);
		if (len <= 0)
		{
			InfoLog("服务器断开连接");
			m_isRun = false;
			Close();
		}

		ProtoHead* head = (ProtoHead*)buf;

		recv(m_sock, buf + sizeof(ProtoHead), head->length - sizeof(ProtoHead), 0);
		ProcessMessage(head);
	}

	void ProcessMessage(ProtoHead* message)
	{
		switch (message->cmd_id)
		{
		case cmd_sc_new_clinet:
		{
			SC_NewClient* new_client = (SC_NewClient*)message;
			string info = "新客户端连接 : IP: ";
			info += new_client->ip;
			InfoLog(info);
		}
		break;
		case cmd_sc_disconnect_clinet:
		{
			SC_Disconnect* dis_client = (SC_Disconnect*)message;
			string info = "客户端断开连接连接 : sock: ";
			info += std::to_string(dis_client->sock);
			InfoLog(info);
		}
		break;
		case  cmd_sc_login:
		{
			SC_Login* login = (SC_Login*)message;
			string info = "登录消息返回 : sock: ";
			info += std::to_string(login->result);
			InfoLog(info);
		}
		break;
		default:
			break;
		}

	}

	void SendMess(ProtoHead* message)
	{
		send(m_sock, (char*)message, message->length, 0);
	}
private:
	void InfoLog(const std::string str)
	{
		std::cout << "Info : " << str << std::endl;
	}

	void ErrorLog(const std::string str)
	{
		std::cout << "Error : " << str << std::endl;
	}

private:
	SOCKET m_sock;
	bool m_isRun;
};
