#pragma once
#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#include "pch.h"

#define WAIT_INTERV 1000

#define DATA_PACKET_INITIALIZATION 10
#define DATA_PACKET_INITIALIZATION_CONFIRMATION 11
DLL_EXPORT struct dataPacketInitializer
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


DLL_EXPORT bool SetupReceiveSocket(SOCKET &sock, int port);
DLL_EXPORT bool ConnectTo(SOCKET &sock, char* IpAddress, int port);


DLL_EXPORT int GetNOProc();


DLL_EXPORT bool SetupUDPSocket(SOCKET &clientSocket, int port);

DLL_EXPORT bool Send(SOCKET sock, char* data, int buffLen, int flags);
DLL_EXPORT bool Recv(SOCKET sock, char* data, int flags);

//send(wPoolData->wPool->at(jj)->sock, data, buffLen, NULL
#endif