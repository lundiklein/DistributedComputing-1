#pragma once
#ifndef _NETWORK_CUSTOM_H_
#define _NETWORK_CUSTOM_H_

#include "Network.h"

typedef struct baseRequest
{
	uint8_t actionId; //1-login/2-refreshLobby/3-joinServer/4-logout
	uint32_t NetId;
};
typedef struct registerRQ : baseRequest
{
	char login[16];
	char password[16];
};
typedef struct loginRequest : baseRequest
{
	char login[16];
	char password[16];
};

typedef struct joinWorkerPoolRequest : baseRequest
{
	uint8_t minPriority;
	uint8_t threads;
	uint16_t ram;
};
typedef struct calculateRequest : baseRequest
{
	//SIMULATION PARAMETERS
	uint32_t iterations;
	uint32_t cellNumber;
};
typedef struct workerCalculateRequest : baseRequest
{
	//SIMULATION PARAMETERS
	uint32_t cellNumber;
	uint32_t cellNumberMax;
};
typedef struct cellDataStruct
{
private:
	uint16_t x, y;
	uint8_t state;
public:
	cellDataStruct(uint16_t x, uint16_t y)
	{
		this->x = x;
		this->y = y;
		this->state = 0;
	}
	uint16_t getX()
	{
		return x;
	}
	uint16_t getY()
	{
		return y;
	}
	uint8_t getState()
	{
		return state;
	}
	void setState(uint8_t state)
	{
		this->state = state;
	}
};

struct dataPacket
{
	uint32_t packetSize;
	uint32_t iteration;
	std::vector<cellDataStruct> dataVector;

	char* serializeData(std::vector<cellDataStruct> dataVector, uint32_t iteration, int& buffLen)
	{
		packetSize = dataVector.size();
		this->iteration = iteration;
		for (int ii = 0; ii < packetSize; ++ii)
		{
			this->dataVector.push_back(dataVector[ii]);
		}
		buffLen = sizeof(2 * sizeof(uint32_t) + dataVector.size() * sizeof(cellDataStruct));
		char* data = new char[buffLen];
		dataPacket* dataP = (dataPacket*)data;
		dataP = this;
		return data;
	}
	dataPacket operator=(dataPacket data)
	{
		this->packetSize = data.packetSize;
		this->iteration = data.iteration;
		for (int ii = 0; ii < data.dataVector.size(); ii++)
		{
			this->dataVector.push_back(data.dataVector[ii]);
		}
		return *this;
	}
};


int GetNOProc();
bool ConnectTo(SOCKET &servSocket, char* IpAddress, int port);
bool SetupReceiveSocket(SOCKET &clientSocket, int port);
bool JoinWorkerPool(SOCKET& sock);
#endif