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
		char* data = nullptr;// [sizeof(isServerRequest)];
		uint32_t size = sizeof(isServerRequest);
		if (Recv(sock, data, NULL, true) != SOCKET_ERROR)
		{
			std::cout << "RECV" << std::endl;
			isServerRequest* rq = (isServerRequest*)data;
			rq->actionId = 0;
			rq->response = 1;
			std::cout << "RECV" << std::endl;
			if (Send(sock, data, size, NULL, true))
			{
				
				std::cout << "SEND" << std::endl;
				char servData[sizeof(joinWorkerPoolRequest)];
				joinWorkerPoolRequest* rq2 = (joinWorkerPoolRequest*)servData;
				
				//TMP
				rq2->ram = 12000;
				//TODO => CHECK AVAIABLE CORES
				int p = 1;
				rq2->threads = p;
				rq2->minPriority = 0;
				size = sizeof(servData);
				std::cout << "SEND 2" << std::endl;
				Sleep(10);
				return Send(sock, servData, size, NULL, true);
				
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
