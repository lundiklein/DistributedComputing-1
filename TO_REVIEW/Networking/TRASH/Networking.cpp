#include "Networking.h"


bool Networking::RecLoginData(SOCKET receiveSocket, char *data, int dataLen, int flags, LoginData* rdata)
{
	ZeroMemory(rdata, PACKETSIZE);
	int bytesReceived;
	do
	{
		bytesReceived = recv(receiveSocket, data, dataLen, flags);
		std::cout << bytesReceived << std::endl;
		if (bytesReceived > 0)
		{
			packetManager.Deserialize(data, rdata);
			//--------------------------------------------------
			/*
			LOGIN AUTHORYSATION!
			*/
			//--------------------------------------------------

			packetManager.PrintMessage(rdata);

			return true;

		}
		else if (bytesReceived == -1)
		{
			std::cout << "RECEIVE ERROR: " << WSAGetLastError() << std::endl;
			break;
		}
		else if (bytesReceived == 0)
		{
			std::cout << "CONNECTION CLOSED\n" << std::endl;
			break;
		}
		


	} while (bytesReceived > 0);

	return false;
}
void Networking::Send(SOCKET Socket, char* sendInfo)
{
	std::cout << "SENDING" << std::endl;
	
	if (send(Socket, sendInfo, DEFAULT_BUFLEN - 1, 0) == SOCKET_ERROR)
	{
		std::cout << "SEND SOCKET ERROR " << WSAGetLastError() << std::endl;
	}
	else
	{
		std::cout << ("DATA SENT ") << std::endl;
	}

}

void Networking::ConnectTo(SOCKET &clientSocket, char* IpAddress, int port)
{

	SOCKADDR_IN socketAddress, *chujchuj;
	//struct addrinfo sd, *chuj;
	socketAddress.sin_family = AF_INET;
	
	socketAddress.sin_addr.s_addr = inet_addr(IpAddress);
	
	socketAddress.sin_port = htons(port);
	//sd.ai_family = AF_INET;
	//sd.ai_protocol = IPPROTO_TCP;
	//sd.ai_socktype = SOCK_STREAM;

	//getaddrinfo(LOGIN_SERVER_IP, LOGIN_SERVER_PORT, &sd, &chuj);
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (connect(clientSocket, (SOCKADDR *)&socketAddress, sizeof(socketAddress)) != SOCKET_ERROR)
	{
		printf("CONNECTED TO CHAR SERVER\n");

	}
	else
	{
		printf("FAILED TO CONNECT TO CHAR SERVER\n");
		std::cout << WSAGetLastError() << std::endl;

	}

}

void Networking::SetupUDPSocket(SOCKET &clientSocket, int port)
{

}
void Networking::SetupMulticastSendSocket(SOCKET &clientSocket, char* multicastIP, int multicastPort, SOCKADDR_IN &multicastAddr)
{

	clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("MULTICAST SOCKET ERROR: %d ", WSAGetLastError());
	}
	memset(&multicastAddr, 0, sizeof(multicastAddr));
	multicastAddr.sin_family = AF_INET;
	multicastAddr.sin_addr.s_addr = inet_addr(multicastIP);
	multicastAddr.sin_port = multicastPort;

}
void Networking::SetupReceiveMCSocket(SOCKET &clientSocket, char* multicastIP, int multicastPort, SOCKADDR_IN &multicastAddr, ip_mreq &mcJoin)
{
	int opt = 1;
	clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (clientSocket = INVALID_SOCKET)
	{
		printf("MC RECV SOCKET ERROR %d \n", WSAGetLastError());
	}
	if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR)
	{
		printf("MC SOCKET OPTION ERROR %d \n", WSAGetLastError());
	}
	memset(&multicastAddr, 0, sizeof(multicastAddr));
	multicastAddr.sin_family = AF_INET;
	multicastAddr.sin_addr.s_addr = inet_addr(multicastIP);
	multicastAddr.sin_port = multicastPort;
	if (bind(clientSocket, (SOCKADDR*)&multicastAddr, sizeof(multicastAddr)) == SOCKET_ERROR)
	{
		printf("MC BIND ERROR %d \n", WSAGetLastError());
	}
	mcJoin.imr_multiaddr.s_addr = inet_addr(multicastIP);
	mcJoin.imr_interface.s_addr = INADDR_ANY;
	if (setsockopt(clientSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcJoin, sizeof(mcJoin)) == SOCKET_ERROR)
	{
		printf("MC JOIN GROUP ERROR %d \n", WSAGetLastError());
	}

}
void Networking::ConnectedClients(SOCKET clientSocket, SOCKADDR_IN peerAddr, std::list<ClientList>& cl, int& id, Scene scene)
{
	int plen = sizeof(peerAddr);
	if (getpeername(clientSocket, (sockaddr*)&peerAddr, &plen) == SOCKET_ERROR)
	{
		printf("GET PEER ADDR ERROR: %d \n", WSAGetLastError());
	}
	else
	{
		//printf("PEER ADDR ADDED");
		ClientList list;
		//clients = inet_ntoa(peerAddr.sin_addr);
		list.id = id;
		list.clientIP = inet_ntoa(peerAddr.sin_addr);
		list.scene = scene;
		cl.push_back(list);
		//printf("Peer's IP address is: %s\n", inet_ntoa(peerAddr.sin_addr));
		//std::cout << clientList.size()<<" "<<clients << std::endl;
		id++;


	}
}
void Networking::PrintConnectedClientsList(std::list<ClientList> clientList)
{

	std::list<ClientList>::iterator i;
	for (i = clientList.begin(); i != clientList.end(); ++i)
	{
		std::cout << "ID: " << i->id <<" IP: "<< i->clientIP << " SCENE: " << i->scene<< std::endl;
	}

}
int GetNOProcesors()
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

bool SetupReceiveSocket(SOCKET &clientSocket, int port)
{
	SOCKADDR_IN loginServerInfo;
	memset(&loginServerInfo, 0, sizeof(loginServerInfo));
	loginServerInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	loginServerInfo.sin_port = htons(port);
	loginServerInfo.sin_family = AF_INET;

	if ((clientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("WSASocket CREATION ERROR: %d", WSAGetLastError());
		return false;
	}
	else
	{
		printf("WSASocket CREATION SUCCESS");
	}
	if (bind(clientSocket, (SOCKADDR*)&loginServerInfo, sizeof(loginServerInfo)) == SOCKET_ERROR)
	{
		printf("WSASocket BIND ERROR: %d", WSAGetLastError());
		return false;
	}
	else
	{
		printf("WSASocket BIND SUCCESS");
	}
	if (listen(clientSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("WSASocket LISTEN ERROR: %d", WSAGetLastError());
		return false;
	}
	else
	{
		printf("WSASocket LISTEN SUCCESS");
	}
	return true;
}
bool Init()
{
	maxThreads = NO_OF_THREADS_PER_PROCESOR * GetNOProcesors();
	h_WORKER_THREADS = new HANDLE[maxThreads];

	WSAData wsadat;
	if (WSAStartup(MAKEWORD(2, 2), &wsadat) != 0)
	{
		printf("WSA STARTUP ERROR: %d\n", WSAGetLastError());
		return false;
	}
	else
	{
		printf("WSA STARTUP SUCCESS\n");
	}
	if (!InitIOCP())
	{
		return false;
	}
	else
	{
		printf("INIT IOCP SUCCESS\n");
	}
	return true;
}
bool InitIOCP()
{
	h_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (h_IOCP = NULL)
	{
		printf("INIT IOCP ERROR: %d\n", GetLastError());
		return false;
	}
	DWORD numb;
	for (int ii = 0; ii < maxThreads; ii++)
	{
		h_WORKER_THREADS[ii] = CreateThread(0, 0, WorkerThread, (void*)(ii + 1), 0, &numb);
	}
	return true;
}
void AcceptConnections(SOCKET clientSocket)
{
	SOCKADDR_IN clientInfo;
	int clInfoLen = sizeof(clientInfo);
	SOCKET receiveSocket = INVALID_SOCKET;
	if ((receiveSocket = accept(clientSocket, (sockaddr *)&clientInfo, &clInfoLen)) == INVALID_SOCKET)
	{
		printf("ACCEPT CLIENT ERROR: %d\n", WSAGetLastError());
	}
	else
	{
		printf("CLIENT IP: %d\n", inet_ntoa(clientInfo.sin_addr));
	}
	ClientInfo *clinfo = new ClientInfo;
	clinfo->SetOPT(OPT_READ);
	clinfo->SetSOCKET(receiveSocket);
	if (ConnectWithIOCP(clinfo))
	{
		//Once the data is successfully received, we will print it.
		clinfo->SetOPT(OPT_WRITE);

		WSABUF *p_wbuf = clinfo->GetWSABUF();
		OVERLAPPED *p_ol = clinfo->GetOVERLAPPED();

		//Get data.
		DWORD dwFlags = 0;
		DWORD dwBytes = 0;

		//Post initial Recv
		//This is a right place to post a initial Recv
		//Posting a initial Recv in WorkerThread will create scalability issues.
		int nBytesRecv = WSARecv(clinfo->GetSOCKET(), p_wbuf, 1,
			&dwBytes, &dwFlags, p_ol, NULL);

		if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
		{
			printf("\nError in Initial Post.");
		}
	}
		



}
bool ConnectWithIOCP(ClientInfo* clinfo)
{
	HANDLE hle = CreateIoCompletionPort((HANDLE)clinfo->GetSOCKET(), h_IOCP, (DWORD)clinfo, 0);
	if (hle == NULL)
	{
		printf("CONNECT WITH IOCP ERROR: %d\n", GetLastError());
		return false;
	}
	return true;
}
bool AcceptEvent(SOCKET socket, HANDLE &handle)
{
	handle = WSACreateEvent();
	if (handle == WSA_INVALID_EVENT)
	{
		printf("ACCEPT EVENT INVALID HANDLE ERROR: %d\n", WSAGetLastError());
		return false;
	}
	if (WSAEventSelect(socket, handle, FD_ACCEPT) == SOCKET_ERROR)
	{
		printf("ACCEPT EVENT SELECT ERROR: %d\n", WSAGetLastError());
		WSACloseEvent(handle);
		return false;
	}
	DWORD nThreads;
	handle = CreateThread(0, 0, AcceptThread, (void*)socket, 0, &nThreads);
}

DWORD WINAPI AcceptThread(LPVOID param)
{
	SOCKET Listener = (SOCKET)param;
	WSANETWORKEVENTS netEv; 

	while (WAIT_OBJECT_0 != WaitForSingleObject(h_SHUTDOWN, 0))
	{
		if (WSAWaitForMultipleEvents(1, &h_ACCEPT_EVENT, FALSE, WAIT_INTERV, FALSE))
		{
			WSAEnumNetworkEvents(Listener, h_ACCEPT_EVENT, &netEv);
		}
		if ((netEv.lNetworkEvents & FD_ACCEPT) && (0 == netEv.iErrorCode[FD_ACCEPT_BIT]))
		{
			AcceptConnections(Listener);
		}
	}
	return 0;
}
DWORD WINAPI WorkerThread(LPVOID param)
{
	int nThreadNo = (int)param;

	void *lpContext = NULL;
	OVERLAPPED       *pOverlapped = NULL;
	ClientInfo   *clientInfo = NULL;
	DWORD            dwBytesTransfered = 0;
	int nBytesRecv = 0;
	int nBytesSent = 0;
	DWORD             dwBytes = 0, dwFlags = 0;

	//Worker thread will be around to process requests, until a Shutdown event is not Signaled.
	while (WAIT_OBJECT_0 != WaitForSingleObject(h_SHUTDOWN, 0))
	{
		BOOL bReturn = GetQueuedCompletionStatus(
			h_IOCP,
			&dwBytesTransfered,
			(LPDWORD)&lpContext,
			&pOverlapped,
			INFINITE);

		if (NULL == lpContext)
		{
			//We are shutting down
			break;
		}

		//Get the client context
		clientInfo = (ClientInfo *)lpContext;

		if ((FALSE == bReturn) || ((TRUE == bReturn) && (0 == dwBytesTransfered)))
		{
			//Client connection gone, remove it.
			//RemoveFromClientListAndFreeMemory(pClientContext);
			continue;
		}

		WSABUF *p_wbuf = clientInfo->GetWSABUF();
		OVERLAPPED *p_ol = clientInfo->GetOVERLAPPED();

		switch (clientInfo->GetOPT())
		{
		case OPT_READ:

			
			break;

		case OPT_WRITE:

			break;

		default:
			//We should never be reaching here, under normal circumstances.
			break;
		} // switch
	} // while

	return 0;
}