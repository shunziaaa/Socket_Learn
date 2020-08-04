#include "Server.h"
int main()
{
	Server* ser = new Server();
	ser->Init("127.0.0.1", 8888, 5);
	while (ser->IsRun())
	{
		ser->Run();
	}

	ser->Close();
	delete ser;
	return 0;
}