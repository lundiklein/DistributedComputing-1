#pragma once
#ifndef _NETWORK_H_
#define _NETWORK_H_

#include "ServerOptions\ServerOptions.h"
#define WAIT_INTERV 1000

#define DATA_PACKET_INITIALIZATION 10
#define DATA_PACKET_INITIALIZATION_CONFIRMATION 11
struct dataPacketInitializer
{
	uint8_t action;
	uint32_t packetSize;

	dataPacketInitializer(uint8_t action, uint32_t packetSize)
	{
		this->action = action;
		this->packetSize = packetSize;
	}
	//CHECK
	dataPacketInitializer operator=(dataPacketInitializer &data)
	{
		this->action = data.action;
		this->packetSize = data.packetSize;
		return *this;
	}
	char* serialize()
	{
		char* packet = new char[sizeof(dataPacketInitializer)];
		dataPacketInitializer* transfer = (dataPacketInitializer*)packet;
		*transfer = *this;
		return packet;
	};
	void deserialize(char* data) 
	{ 
		dataPacketInitializer* transfer = (dataPacketInitializer*)data;
		*this = *transfer;
	};
};


bool SetupReceiveSocket(SOCKET &sock, int port);
bool ConnectTo(SOCKET &sock, char* IpAddress, int port);


int GetNOProc();


bool SetupUDPSocket(SOCKET &clientSocket, int port);

bool Send(SOCKET sock, char* data, int buffLen, int flags);
bool Recv(SOCKET sock, char* data, int flags);

//send(wPoolData->wPool->at(jj)->sock, data, buffLen, NULL
#endif