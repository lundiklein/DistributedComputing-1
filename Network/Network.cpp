#include "stdafx.h"
#include "Network.h"

#pragma warning(disable:4996)

int TransferMessage(SOCKET sock, char* data, int buffLen, int flags, bool isSending)
{
	int result = -1;
	if (data == nullptr)
	{
		data = new char[buffLen];
		ZeroMemory(data, buffLen);
	}
	if (isSending)
	{
		result = send(sock, data, buffLen, flags);
		while (result == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
			//	printf("Transfer message -> Send error %d\n", WSAGetLastError());
				//system("Pause");
				return result;
			}
			//printf("Transfer message S -> Wouldblock %d\n", WSAGetLastError());
			Sleep(1);
			result = send(sock, data, buffLen, flags);
		}
	}
	else
	{
		result = recv(sock, data, buffLen, flags);
		while (result == SOCKET_ERROR)
		{

			ZeroMemory(data, buffLen);
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
			//	printf("Transfer message -> Recv error %d\n", WSAGetLastError());
			//	system("Pause");
				return result;
			}
			//printf("Transfer message R -> Wouldblock %d\n", WSAGetLastError());
			Sleep(1);
			result = recv(sock, data, buffLen, flags);
		}
	}
	return result;
}
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

bool Send(SOCKET sock, char* data, int buffLen, int flags, bool shouldConfirm)
{

	//printf("Send start\n");
	networkDataPacket* packet = new networkDataPacket(DATA_PACKET_TRANSFER_START, 0, buffLen, buffLen, 0, nullptr);

	uint32_t fullDataSize = buffLen;// +(buffLen / packet->getMaxBufferLength() + 1) * sizeof(dataPacketHeader);
	packet->header.fullDataSize = fullDataSize;
	char* dat = nullptr;
	if (shouldConfirm)
	{
		dat = packet->serialize();
	//	std::cout << "ACTION STEST " << (int)packet->header.action << std::endl;
		if (!TransferMessage(sock, dat, packet->getBufferLength(), NULL, true))
		{
			printf("Send1 error %d\n", WSAGetLastError());
			return TransferMessage(sock, dat, packet->getBufferLength(), NULL, true);
		}
		//delete[] dat;
		//dat = nullptr;
	//	std::cout << "SEND 14" << std::endl;
		//dat = new char[packet->getMaxBufferLength()];
		//ZeroMemory(dat, packet->getMaxBufferLength());

		if (TransferMessage(sock, dat, packet->getBufferLength(), NULL, false))
		{
		//	std::cout << "SEND 15" << std::endl;
			packet->deserialize(dat);

		//	std::cout << "ACTION STEST2 " << (int)packet->header.action << std::endl;
			if (packet->header.action != DATA_PACKET_TRANSFER_CONFIRMATION)
			{
				std::cout << "SEND 15e " << (int)packet->header.action << std::endl;
				system("Pause");
				delete[] dat;
				delete packet;
				//packet->header.action = DATA_PACKET_TRANSFER_ABORT;
				//packet->header.fullDataSize = 0;
				//dat = packet->serialize();

				//TransferMessage(sock, dat, packet->getBufferLength(), NULL, true);
			//	printf("Send error end\n");
				return false;
			}
		}
		else
		{
		//	std::cout << "SEND 14E" << std::endl;
			return false;
		}
	}

	//delete packet;
	//------------------------
	uint32_t packetId = 1;
	int counter = 0;
	if (dat != nullptr)
	{
		delete[] dat;
		dat = nullptr;
	}
	while (packet->header.action != DATA_PACKET_TRANSFER_FINISH)
	{

		if (buffLen - counter > MAX_PACKET_DATA_SIZE)
		{
			packet->setNetworkDataPacket(DATA_PACKET_TRANSFER, packetId, MAX_PACKET_DATA_SIZE, buffLen, 1, &data[counter]);
			if (dat == nullptr)
			{
				dat = new char[packet->getMaxBufferLength()];
			}
			
			packet->serialize(dat);
			int result = TransferMessage(sock, dat, packet->getMaxBufferLength(), NULL, true);
			if (result > sizeof(dataPacketHeader))
			{
				counter += result -sizeof(dataPacketHeader);
			}
			else
			{
				delete[] dat;
				packetId--;
				//delete packet;
				//return false;
			}
		}
		else
		{
			packet->setNetworkDataPacket(DATA_PACKET_TRANSFER_FINISH, packetId, buffLen - counter, buffLen, 1, &data[counter]);
			if (dat != nullptr)
			{
				delete[] dat;
				dat = nullptr;
			}
			dat = new char[buffLen - counter + packet->getMinBufferLength()];
			packet->serialize(dat);
			int result = TransferMessage(sock, dat, buffLen - counter + packet->getMinBufferLength(), NULL, true);
			if (result > sizeof(dataPacketHeader))
			{
				counter += result - sizeof(dataPacketHeader);
			}
			else
			{
				delete[] dat;
				packetId--;
				//delete packet;
				//return false;
			}
		}
		packetId++;

	}

//	std::cout << "SEND 17" << std::endl;

	//printf("Send end\n");
	return true;
}
int Recv(SOCKET sock, char* &data, int flags, bool shouldConfirm, int dataBufferLen)
{
//	printf("Recv start\n");
	uint16_t buffLen = 0;
	uint32_t fullDataSize = 0;
	networkDataPacket* packet = new networkDataPacket(DATA_PACKET_TRANSFER_START, 0, 0, 0, 0, nullptr);
	//dataPacketInitializer dpi = dataPacketInitializer(DATA_PACKET_INITIALIZATION, buffLen);
	char* dat = nullptr;
	if (shouldConfirm)
	{
		dat = new char[packet->getBufferLength()];
		//std::cout << "Recv 1 " << std::endl;
		if (TransferMessage(sock, dat, packet->getBufferLength(), NULL, false))
		{
			packet->deserialize(dat);
		//	std::cout << "ACTION TEST " << (int)packet->header.action << std::endl;
		//	std::cout << "RCV 13 " << (int)packet->header.dataSize << std::endl;
			if (packet->header.action == DATA_PACKET_TRANSFER_START)
			{
				buffLen = packet->header.dataSize;
				fullDataSize = packet->header.fullDataSize;
				packet->setNetworkDataPacket(DATA_PACKET_TRANSFER_CONFIRMATION, 0, 0, 0, 0, nullptr);
			//	std::cout << "ACTION TEST2 " << (int)packet->header.action << std::endl;
				delete[] dat;
				dat = packet->serialize();
				TransferMessage(sock, dat, packet->getBufferLength(), NULL, true);
				delete[] dat;
				dat = nullptr;
			}
		}
	//	std::cout << "Recv 2 " << buffLen << std::endl;

	}
	char* packetData = nullptr;
	int counter = 0;
	uint32_t prevPacketId = 0;
	if (data != nullptr)
	{
		delete[] data;
	}
	if (fullDataSize != 0)
	{
		packetData = new char[fullDataSize];
	}
	if (dat == nullptr)
	{
		dat = new char[packet->getMaxBufferLength()];
		ZeroMemory(dat, packet->getMaxBufferLength());
	}
	uint32_t dataSizeCheck = 0;
	std::vector<networkDataPacket*> missIdPacketVector;
	while (packet->header.action != DATA_PACKET_TRANSFER_FINISH && (packet->header.action == DATA_PACKET_TRANSFER ||
		packet->header.action == DATA_PACKET_TRANSFER_CONFIRMATION) || (dataSizeCheck < packet->header.fullDataSize && packet->header.isDataPacket == 1))
	{
		int result = TransferMessage(sock, dat, packet->getMaxBufferLength(), NULL, false);

		packet->deserialize(dat);
		if (packet->header.action == DATA_PACKET_TRANSFER_ABORT)
		{
			for (std::vector< networkDataPacket* >::iterator it = missIdPacketVector.begin(); it != missIdPacketVector.end(); ++it)
			{
				delete (*it);
			}
			missIdPacketVector.clear();

			//uint32_t dataSize = packet->header.fullDataSize;
			delete dat;
			delete packet;
			return -1;
		}
		if (result > 0)
		{
			if (packetData == nullptr)
			{
			//	std::cout << "FPSIZE " << packet->header.fullDataSize << std::endl;
				packetData = new char[packet->header.fullDataSize];
				ZeroMemory(packetData, packet->header.fullDataSize);
				//system("Pause");
			}
			if (packet->header.isDataPacket == 1)
			{
				
				if (prevPacketId + 1 != packet->header.packetId)
				{
					
					//	system("Pause");
					//	return -1;
					missIdPacketVector.push_back(new networkDataPacket(*packet));
					for (int ii = 0; ii < missIdPacketVector.size(); ++ii)
					{
						if (prevPacketId + 1 == missIdPacketVector[ii]->header.packetId)
						{
							*packet = *missIdPacketVector[ii];
							delete missIdPacketVector[ii];
							missIdPacketVector.erase(missIdPacketVector.begin() + ii);
							break;
						}
						
					}
					std::cout << "PACKET ID MISSMATCH " << prevPacketId + 1 << " / " << (int)packet->header.packetId << std::endl;
				}
				for (int ii = 0; ii < packet->header.dataSize; ++ii)
				{
					packetData[ii + counter] = packet->data[ii];
				}
				counter += packet->header.dataSize;
			//	std::cout << "DATA SIZE " << packet->header.dataSize << " CNTR " << counter << " PACKET ID: " << prevPacketId + 1 << " / " << (int)packet->header.packetId  << " RES " << result << std::endl;
				prevPacketId = packet->header.packetId;
				dataSizeCheck += result - sizeof(dataPacketHeader);

			}
		}
		
		/*
		if (counter > dataBufferLen && dataBufferLen != 0)
		{
			//delete[] data;
			//data = nullptr;
			delete dat;
			delete packet;
			return -1;
		}
		*/
	}
	for (std::vector< networkDataPacket* >::iterator it = missIdPacketVector.begin(); it != missIdPacketVector.end(); ++it)
	{
		delete (*it);
	}
	missIdPacketVector.clear();
	
	//uint32_t dataSize = packet->header.fullDataSize;
	delete dat;
	delete packet;
	//std::cout << "Recv 3 " << std::endl;
	data = packetData;

	//printf("Recv end\n");
	return packet->header.fullDataSize;
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
