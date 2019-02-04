#include "stdafx.h"
#include "networking.h"

bool setupTCPSocket(SOCKET* socket)
{

	return true;
}
bool tcpConnect(SOCKET* clientSocket, std::string addr, int port)
{
	debugMessage("Connecting");
	SOCKADDR_IN socketAddress;
	SOCKADDR_IN clientAddress;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("PORT: %d"), port));

	ZeroMemory((char*)&clientAddress, sizeof(SOCKADDR_IN));
	clientAddress.sin_family = AF_INET;
	clientAddress.sin_port = htons(port);

	ZeroMemory((char*)&socketAddress, sizeof(SOCKADDR_IN));
	//socketAddress.sin_family = AF_INET;

	//inet_pton(AF_INET, IpAddress, &socketAddress.sin_addr.s_addr);
	//socketAddress.sin_port = htons(port);

	//----------------------------------------------------------------------------------------------------------------
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char Temp[5];
	sprintf(Temp, "%i", port);
	unsigned long dwRetval;
	//resolveHostName(SERVER_ADDR, &(socketAddress.sin_addr));
	dwRetval = getaddrinfo(addr.c_str(), Temp, &hints, &result);
	if (dwRetval != 0)
	{
		debugMessage("ERROR: %d", dwRetval);
	}
	socketAddress = *(SOCKADDR_IN*)result->ai_addr;
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(port);

	//----------------------------------------------------------------------------------------------------------------
	BOOL bOptVal = 1;
	int bOptLen = sizeof(BOOL);

	*clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//JUST FOR NO REMOTE SERVER PHASE -> B4 RELEASE DELETE IT/
	if (setsockopt(*clientSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&bOptVal, bOptLen) == SOCKET_ERROR)
	{
		debugMessage(("ERROR - Setsockopt(SO_REUSEADDR): %d"), WSAGetLastError());
	}
	if (bind(*clientSocket, (sockaddr*)&clientAddress, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		debugMessage(("ERROR - Bind socket: %d"), WSAGetLastError());
	}
	if (connect(*clientSocket, (sockaddr *)&socketAddress, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		debugMessage(("ERROR - Connect to: %d"), WSAGetLastError());
		closesocket(*clientSocket);
		return false;
	}
	return true;
}