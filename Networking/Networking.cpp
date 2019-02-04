#include "pch.h"
#include "Networking.h"

#pragma warning(disable:4996)

bool SetupReceiveSocket(SOCKET &sock, int port)
{
	SOCKADDR_IN socketAddr;
	ZeroMemory(&socketAddr, sizeof(socketAddr));
	socketAddr.sin_addr.s_addr = INADDR_ANY;
	socketAddr.sin_port = htons(port);
	socketAddr.sin_family = AF_INET;


	if ((sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("WSASocket CREATION ERROR: %d\n", WSAGetLastError());
		return false;
	}
	else
	{
		printf("WSASocket CREATION SUCCESS\n");
	}
	if (bind(sock, (SOCKADDR*)&socketAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		printf("WSASocket BIND ERROR: %d\n", WSAGetLastError());
		return false;
	}
	else
	{
		printf("WSASocket BIND SUCCESS ON PORT %d\n", ntohs(socketAddr.sin_port));
	}
	if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("WSASocket LISTEN ERROR: %d\n", WSAGetLastError());
		return false;
	}
	else
	{
		printf("WSASocket LISTEN SUCCESS\n");
	}
	return true;

}
bool ConnectTo(SOCKET &sock, char* IpAddress, int port)
{

	SOCKADDR_IN socketAddress;
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_addr.s_addr = inet_addr(IpAddress);
	socketAddress.sin_port = htons(port);

	if ((sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("WSASocket CREATION ERROR: %d\n", WSAGetLastError());
		return false;
	}
	else
	{
		printf("WSASocket CREATION SUCCESS\n");
	}
	if (connect(sock, (SOCKADDR *)&socketAddress, sizeof(socketAddress)) != SOCKET_ERROR)
	{
		printf("CONNECTED TO %s\n", IpAddress);
	}
	else
	{
		closesocket(sock);
		printf("CONNECT TO %s ERROR: %d\n", IpAddress, WSAGetLastError());
		return false;
	}
	return true;
}

bool Send(SOCKET sock, char* data, int buffLen, int flags)
{
	int counter = 0;
	dataPacketInitializer dpi = dataPacketInitializer(DATA_PACKET_INITIALIZATION, buffLen);
	char* dat = dpi.serialize();

	//ZeroMemory(dat, sizeof(dat));
	//dataPacketInitializer* dpi = (dataPacketInitializer*)dat;
	//dpi->action = DATA_PACKET_INITIALIZATION;
	//dpi->packetSize = buffLen;


	if (send(sock, dat, sizeof(sizeof(dataPacketInitializer)), NULL) == SOCKET_ERROR)
	{
		printf("Send error %d\n", WSAGetLastError());
		return Send(sock, data, buffLen, flags);
	}
	if (recv(sock, dat, sizeof(sizeof(dataPacketInitializer)), NULL) != SOCKET_ERROR)
	{
		dpi.deserialize(dat);
		if (dpi.action != DATA_PACKET_INITIALIZATION_CONFIRMATION)
			return false;
	}
	else
		return false;

	if (buffLen > 1024)
	{
		while (counter < buffLen)
		{
			char sendBuffer[1024];
			for (int ii = 0; ii < 1024; ++ii)
			{
				sendBuffer[ii] = *data;
				data++;
				counter++;
			}
			while (send(sock, sendBuffer, 1024, NULL) == SOCKET_ERROR)
			{
				printf("Send error %d\n", WSAGetLastError());
			}
		}
	}
	else
	{
		char* sendBuffer = new char[buffLen];
		for (int ii = 0; ii < buffLen; ++ii)
		{
			sendBuffer[ii] = *data;
			data++;
			counter++;
		}
		while (send(sock, sendBuffer, buffLen, NULL) == SOCKET_ERROR)
		{
			printf("Send error %d\n", WSAGetLastError());
		}
	}
}
bool Recv(SOCKET sock, char* data, int flags)
{
	int counter = 0;
	int buffLen = 0;
	dataPacketInitializer dpi = dataPacketInitializer(0, 0);
	char* dat = new char[sizeof(dataPacketInitializer)];
	if (recv(sock, dat, sizeof(dataPacketInitializer), NULL) != SOCKET_ERROR)
	{
		dpi.deserialize(dat);
		if (dpi.action == DATA_PACKET_INITIALIZATION)
		{
			dpi.action = DATA_PACKET_INITIALIZATION_CONFIRMATION;
			buffLen = dpi.packetSize;
			dat = dpi.serialize();
			if (send(sock, dat, sizeof(sizeof(dataPacketInitializer)), NULL) == SOCKET_ERROR)
			{
				printf("Send error %d\n", WSAGetLastError());
				return Send(sock, data, buffLen, flags);
			}
		}
	}
	else
		return false;
	if (buffLen == 0)
		return false;

	while (counter < buffLen)
	{
		char sendBuffer[1024];
		if (recv(sock, sendBuffer, 1024, NULL) == SOCKET_ERROR)
		{
			for (int ii = 0; ii < 1024 && counter < buffLen; ++ii)
			{
				sendBuffer[ii] = *data;
				data++;
				counter++;
			}
		}
		else
		{
			printf("Send error %d\n", WSAGetLastError());
		}
	}
}




bool SetupUDPSocket(SOCKET &clientSocket, int port)
{
	SOCKADDR_IN socketAddr;
	ZeroMemory(&socketAddr, sizeof(socketAddr));
	socketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	socketAddr.sin_port = htons(port);
	socketAddr.sin_family = AF_INET;
	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);


	if ((clientSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("WSASocket CREATION ERROR: %d\n", WSAGetLastError());
		return false;
	}
	else
	{
		printf("WSASocket CREATION SUCCESS\n");
	}
	if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&bOptVal, bOptLen) == SOCKET_ERROR)
	{
		printf("SET OPT ERROR: %d\n", WSAGetLastError());
		return false;
	}
	else
	{
		printf("SET OPT SUCCESS\n");
	}
	if (bind(clientSocket, (SOCKADDR*)&socketAddr, sizeof(socketAddr)) == SOCKET_ERROR)
	{
		printf("WSASocket BIND ERROR: %d\n", WSAGetLastError());
		return false;
	}
	else
	{
		printf("WSASocket BIND SUCCESS\n");
	}
	return true;

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
