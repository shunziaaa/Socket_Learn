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

// �¿ͻ��˼���
void OnConnect()
{
	sockaddr_in cli_addr;
	int len = sizeof(sockaddr_in);
	SOCKET client_socket = accept(_sock, (sockaddr*)&cli_addr, &len);
	if (client_socket == INVALID_SOCKET)
	{
		ErrorLog("client_socket connect error");
		return;
	}

	string info = "�¿ͻ�������: IP:";
	info += inet_ntoa(cli_addr.sin_addr);
	info += " socket: ";
	info += std::to_string(client_socket);
	InfoLog(info);
	connect_set.insert(client_socket);

	// ֪ͨ���������û�
	for (auto sock: connect_set)
	{
		if (sock != _sock)
		{
			SC_NewClient response = {};
			response.ip = inet_ntoa(cli_addr.sin_addr);
			send(sock, (char*)&response, sizeof(SC_NewClient), 0);
		}
	}
}

// �ͻ��˶Ͽ�����
void OnDisconnect(SOCKET cli_socket)
{
	if (connect_set.count(cli_socket) == 0)
	{
		return;
	}

	connect_set.erase(cli_socket);

	string info = "�ͻ��˶Ͽ�����: socket:";
	info += std::to_string(cli_socket);
	InfoLog(info);

	// ֪ͨ���������û�
	for (auto sock : connect_set)
	{
		if (sock != _sock)
		{
			SC_Disconnect response = {};
			response.sock = (int)cli_socket;
			send(sock, (char*)&response, sizeof(SC_Disconnect), 0);
		}
	}
}

// �ͻ��˷�����Ϣ
void OnRecvMessage(SOCKET cli_socket)
{
	char buf[1024];
	int len = recv(cli_socket, buf, sizeof(ProtoHead), 0);
	if (len < 0)
	{
		ErrorLog("������Ϣ����");
		return;
	}
	if (len == 0)
	{
		OnDisconnect(cli_socket);
		return;
	}

	ProtoHead* head = (ProtoHead*)buf;
	switch (head->cmd_id)
	{
	case cmd_cs_login:
	{
		recv(cli_socket, buf + sizeof(ProtoHead), sizeof(CS_Login) - sizeof(ProtoHead), 0);
		CS_Login* cs_login = (CS_Login*)buf;
		string info = "���տͻ�����Ϣ CS_Login name: ";
		info += cs_login->name;
		info += "password: ";
		info += cs_login->passward;
		InfoLog(info);
	}
	break;
	default:
		InfoLog("���մ�����Ϣ");
		break;
	}
}





int main()
{
	// ��������
	WORD w = MAKEWORD(2, 2);
	WSAData data;
	int ret = WSAStartup(w, &data);
	if (ret == INVALID_SOCKET)
	{
		ErrorLog("��������������ʧ��");
		return 0;
	}
	InfoLog("��������������");
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

	ret = bind(_sock, (sockaddr*)&ser_addr, sizeof(sockaddr_in));
	if (ret == SOCKET_ERROR)
	{
		ErrorLog("socket error");
		return 0;
	}

	listen(_sock, 5);
	InfoLog("�ȴ��ͻ�������");
	while (true)
	{
		InitFD();
		timeval t = { 1, 0 };
		ret = select((int)_sock, &read_fd, &write_fd, &ex_fd, &t);
		if (ret < 0)
		{
			ErrorLog("select error");
			return 0;
		}
		
		for (size_t i  = 0; i < read_fd.fd_count; i++)
		{
			if (read_fd.fd_array[i] == _sock)
			{
				OnConnect();
			}

			// ��Ϣ
		}

		InfoLog("ping");
	}

	closesocket(_sock);
	WSACleanup();
	return 0;
}