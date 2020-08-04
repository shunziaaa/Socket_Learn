#include "Client.h"
#include <stdio.h>
#include <thread>

void input(Client * client)
{
	string cmd;
	while (std::cin >> cmd)
	{
		if (cmd == "exit")
		{
			return;
		}
		else if (cmd == "login")
		{
			CS_Login* login = new CS_Login();
			login->name = "name";
			login->passward = "123456";
			client->SendMess(login);
		}
		else
		{
			printf("无效命令\n");
		}
	}
}

int main()
{
	Client* client = new Client();
	client->Init();
	while (!client->Connect("127.0.0.1", 8888))
	{
		printf("正在连接服务器。。。\n");
	} 
	std::thread th(input, client);
	th.detach();

	while (client->IsRun())
	{
		client->Run();
	}

	client->Close();
	delete client;
	system("pause");
	return 0;
}