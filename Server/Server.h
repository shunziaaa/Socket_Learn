#include <iostream>
#include <windows.h>
#include <WinSock2.h>
#include <unordered_set>
#include <string>

#include "proto.h"

using std::unordered_set;
using std::string;

class Server
{
public:
	Server()
	{
		m_socket = INVALID_SOCKET;
		m_socketPool.clear();
		m_isRun = false;
	}

	virtual ~Server()
	{
		Close();
	}

	bool Init(string ip, int port, int lis_count)
	{
		if (m_socket != INVALID_SOCKET)
		{
			return true;
		}
		WORD w = MAKEWORD(2, 2);
		WSAData data;
		int ret = WSAStartup(w, &data);
		if (ret == INVALID_SOCKET)
		{
			ErrorLog("服务器环境开启失败");
			return false;
		}
		InfoLog("服务器环境开启");
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_socket == INVALID_SOCKET)
		{
			ErrorLog("socket error");
			return false;
		}

		sockaddr_in ser_addr = {};
		ser_addr.sin_family = AF_INET;
		ser_addr.sin_port = htons(short(port));
		ser_addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());

		ret = bind(m_socket, (sockaddr*)&ser_addr, sizeof(sockaddr_in));
		if (ret == SOCKET_ERROR)
		{
			ErrorLog("socket error");
			return false;
		}

		listen(m_socket, lis_count);
		InfoLog("等待客户端连接");

		m_isRun = true;
		return true;
	}

	bool IsRun()
	{
		return m_isRun;
	}

	void Run()
	{
		if (!IsRun())
		{
			return;
		}

		ResetFdset();
		timeval t = { 1, 0 };
		select((int)m_socket, &m_read, &m_write, &m_ex, &t);

		// 没有消息
		if (m_read.fd_count <= 0)
		{
			InfoLog("ping...");
			return;
		}

		for (size_t i = 0; i < m_read.fd_count; i++)
		{
			RecvMessage(m_read.fd_array[i]);
		}
	}

	void RecvMessage(SOCKET sock)
	{
		if (sock == m_socket)
		{
			OnNewClient();
			return;
		}

		char buf[1024];
		int recv_len = recv(sock, (char*)buf, sizeof(ProtoHead), 0);
		if (recv_len <= 0)
		{
			DisConnect(sock);
			return;
		}
		ProtoHead* head = (ProtoHead*) buf;
		recv(sock, (char*)buf + sizeof(ProtoHead), head->length - sizeof(ProtoHead), 0);
		ProcessMessage(sock, head);
		head = nullptr;
	}

	void ProcessMessage(SOCKET sock, ProtoHead* head)
	{
		switch (head->cmd_id)
		{
		case cmd_cs_login:
		{
			CS_Login* cs_login = (CS_Login*)head;
			string info = "接收客户端消息 CS_Login name: ";
			info += cs_login->name;
			info += "password: ";
			info += cs_login->passward;
			InfoLog(info);

			SC_Login* sc_login = new SC_Login();
			sc_login->result = 1;
			SendMessage(sock, sc_login);
			//delete sc_login;
		}
		break;
		default:
			InfoLog("接收错误消息");
			break;
		}
	}

	void SendMessage(SOCKET sock, ProtoHead* head)
	{
		send(sock, (char*)head, head->length, 0);
	}

	void Close()
	{
		if (m_socket == INVALID_SOCKET)
		{
			return;
		}
		m_isRun = false;
		closesocket(m_socket);
		m_socketPool.clear();
		WSACleanup();
	}
protected:
	void OnNewClient()
	{
		sockaddr_in cli_addr;
		int len = sizeof(sockaddr_in);
		SOCKET client_socket = accept(m_socket, (sockaddr*)&cli_addr, &len);
		if (client_socket == INVALID_SOCKET)
		{
			return;
		}

		m_socketPool.insert(client_socket);

		string info = "新客户端连接: IP:";
		info += inet_ntoa(cli_addr.sin_addr);
		info += " socket: ";
		info += std::to_string(client_socket);
		InfoLog(info);
	}

	void DisConnect(SOCKET sock)
	{
		if (m_socketPool.count(sock) == 0)
		{
			return;
		}

		m_socketPool.erase(sock);

		string info = "客户端断开连接: socket:";
		info += std::to_string(sock);
		InfoLog(info);

		// 通知所以在线用户

		SC_Disconnect* response = new SC_Disconnect();
		response->sock = (int)sock;
		for (auto c_sock : m_socketPool)
		{
			if (c_sock != m_socket)
			{
				SendMessage(c_sock, response);
			}
		}
		delete response;
	}

private:
	void ResetFdset()
	{
		FD_ZERO(&m_read);
		FD_ZERO(&m_write);
		FD_ZERO(&m_ex);
		FD_SET(m_socket, &m_read);
		for (auto sock : m_socketPool)
		{
			FD_SET(sock, &m_read);
		}
	}

	void InfoLog(const std::string str)
	{
		std::cout << "Info : " << str << std::endl;
	}

	void ErrorLog(const std::string str)
	{
		std::cout << "Error : " << str << std::endl;
	}

private:
	SOCKET m_socket;
	unordered_set<SOCKET> m_socketPool;
	bool m_isRun;
	fd_set m_read;
	fd_set m_write;
	fd_set m_ex;
};