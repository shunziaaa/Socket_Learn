#include <iostream>
#include <windows.h>
#include <WinSock2.h>
#include <unordered_set>
#include <string>

#include "proto.h"

using std::unordered_set;
using std::string;

unordered_set<SOCKET> connect_set;
SOCKET _sock = 0;
fd_set read_fd = {};
fd_set write_fd = {};
fd_set ex_fd = {};

void InfoLog(const std::string str)
{
	std::cout << "Info : " << str << std::endl;
}

void ErrorLog(const std::string str)
{
	std::cout << "Error : " << str << std::endl;
}

// ���ü���fd
void InitFD()
{
	FD_ZERO(&read_fd);
	FD_ZERO(&write_fd);
	FD_ZERO(&ex_fd);
	for (auto sock : connect_set)
	{
		FD_SET(sock, &read_fd);
	}
}

bool RecvMessage()
{
	char buf[1024];
	int len = recv(_sock, buf, sizeof(ProtoHead), 0);
	if (len <= 0)
	{
		InfoLog("�������Ͽ�����");
		return false;
	}

	ProtoHead* head = (ProtoHead*)buf;
	switch (head->cmd_id)
	{
	case cmd_sc_new_clinet:
	{
		recv(_sock, buf + sizeof(ProtoHead), sizeof(SC_NewClient) - sizeof(ProtoHead), 0);
		SC_NewClient* new_client = (SC_NewClient*)buf;
		string info = "�¿ͻ������� : IP: ";
		info += new_client->ip;
		InfoLog(info);
	}
	break;
	case cmd_sc_disconnect_clinet:
	{
		recv(_sock, buf + sizeof(ProtoHead), sizeof(SC_Disconnect) - sizeof(ProtoHead), 0);
		SC_Disconnect* dis_client = (SC_Disconnect*)buf;
		string info = "�ͻ��˶Ͽ��������� : sock: ";
		info += std::to_string(dis_client->sock);
		InfoLog(info);
	}
	break;
	case  cmd_sc_login:
	{
		recv(_sock, buf + sizeof(ProtoHead), sizeof(SC_Login) - sizeof(ProtoHead), 0);
		SC_Login* login = (SC_Login*)buf;
		string info = "��¼��Ϣ���� : sock: ";
		info += std::to_string(login->result);
		InfoLog(info);
	}
	break;
	default:
		break;
	}

	return true;
}

int main()
{
	// ��������
	WORD w = MAKEWORD(2, 2);
	WSAData data;
	int ret = WSAStartup(w, &data);
	if (ret == INVALID_SOCKET)
	{
		ErrorLog("�ͻ��˻�������ʧ��");
		return 0;
	}
	InfoLog("�ͻ��˻�������");

	_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock == INVALID_SOCKET)
	{
		ErrorLog("socket error");
		return 0;
	}

	connect_set.insert(_sock);
	sockaddr_in ser_addr = {};
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(8888);
	ser_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	InfoLog("���ӷ�����...");
	bool flag = false;
	while (not flag)
	{
		int err = connect(_sock, (sockaddr*)&ser_addr, sizeof(sockaddr_in));
		if (err == 0)	flag = true;
	}

	InfoLog("���ӷ������ɹ�...");
	while (true)
	{
		InitFD();
		timeval t = { 1, 0 };

		select((int)_sock, &read_fd, &write_fd, &ex_fd, &t);
		if (FD_ISSET(_sock, &read_fd))
		{
			if (not RecvMessage())
			{
				break;
			}
			continue;
		}

		InfoLog("pong");
	}

	closesocket(_sock);
	WSACleanup();
	system("pause");
	return 0;
}