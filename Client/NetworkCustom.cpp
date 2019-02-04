#include "stdafx.h"
#include "NetworkCustom.h"
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6633

bool JoinWorkerPool(SOCKET& sock)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	sock = INVALID_SOCKET;

	if (ConnectTo(sock, SERVER_IP, SERVER_PORT))
	{
		std::cout << "CONNECTED" << std::endl;
		char* data = nullptr;//char data[sizeof(isServerRequest)];
		int32_t size = sizeof(data);
		if (Recv(sock, data, size, true) != SOCKET_ERROR)
		{
			std::cout << "RECV" << std::endl;
			isServerRequest* rq = (isServerRequest*)data;
			rq->actionId = 0;
			rq->response = 0;
			if (Send(sock, data, size, NULL, true) != SOCKET_ERROR)
			{
				std::cout << "SEND" << std::endl;
				char sdata[sizeof(calculateRequest)];
				calculateRequest* cdat = (calculateRequest*)sdata;
				cdat->actionId = 66;
				cdat->iterations = 100;
				cdat->cellNumber = 500000;
		
				size = sizeof(sdata);

				Sleep(10);
				if (Send(sock, sdata, size, NULL, true) != SOCKET_ERROR)
				{
					return true;
				}
			}
		}

	}

	shutdown(sock, SD_BOTH);
	closesocket(sock);
	WSACleanup();
	return false;
}
int GetNOProc()
{
	static int nProcessors = 0;

	if (0 == nProcessors)
	{
		SYSTEM_INFO si;

		GetSystemInfo(&si);

		nProcessors = si.dwNumberOfProcessors;
	}
	return nProcessors;
}
