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
struct ndataPacket
{
	uint32_t cellNumber;
	uint32_t citeration;
	uint32_t miteration;
	uint32_t healthy;
	uint32_t mutated;
	uint32_t canceroues;
	uint32_t dead;
	uint32_t startX;
	uint32_t startY;
	uint32_t gridSize;

	ndataPacket(uint32_t cellNumber, uint32_t miteration, uint32_t startX, uint32_t startY, uint32_t gridSize)
	{
		this->cellNumber = cellNumber;
		citeration = 0;
		this->miteration = miteration;
		healthy = this->cellNumber;
		mutated = 0;
		canceroues = 0;
		dead = 0;
		this->startX = startX;
		this->startY = startY;
		this->gridSize = gridSize;
	}

	char* serialize()
	{
		char* data = new char[sizeof(ndataPacket)];
		ndataPacket* trf = (ndataPacket*)data;
		*trf = *this;
		return data;
	}
	void deserialize(char* data)
	{
		ndataPacket* trf = (ndataPacket*)data;
		*this = *trf;
	}
	ndataPacket operator=(const ndataPacket& packet)
	{
		this->cellNumber = packet.cellNumber;
		this->citeration = packet.citeration;
		this->miteration = packet.miteration;
		this->healthy = packet.healthy;
		this->mutated = packet.mutated;
		this->canceroues = packet.canceroues;
		this->dead = packet.dead;
		this->startX = packet.startX;
		this->startY = packet.startY;
		this->gridSize = packet.gridSize;
		return *this;
	}
};

struct cellDataStruct
{

	uint16_t x, y, lifeTime;
	uint8_t state, mutationCounter;

public:
	cellDataStruct()
	{
		x = 0;
		y = 0;
		lifeTime = 0;
		state = 0;
		mutationCounter = 0;
	}
	cellDataStruct(uint16_t x, uint16_t y, uint16_t lifeTime, uint8_t state, uint8_t mutationCounter)
	{
		this->x = x;
		this->y = y;
		this->lifeTime = lifeTime;
		this->state = state;
		this->mutationCounter = mutationCounter;
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
	std::vector<cellDataStruct*> data;
	~dataPacket()
	{
		for (std::vector< cellDataStruct* >::iterator it = data.begin(); it != data.end(); ++it)
		{
			delete (*it);
		}
		data.clear();
	}
	char* serialize(int buffLen)
	{
		char* dat = new char[buffLen];
		ZeroMemory(dat, buffLen);
		dataPacket* tt = (dataPacket*)dat;
		uint32_t* tx = (uint32_t*)dat;
		*tx = this->packetSize;
		tx++;
		*tx = this->iteration;
		tx++;
		cellDataStruct* txx = (cellDataStruct*)tx;
		for (int ii = 0; ii < this->data.size(); ++ii)
		{
			cellDataStruct* dt = new cellDataStruct(this->data[ii]->getX(), this->data[ii]->getY(),
				this->data[ii]->lifeTime, this->data[ii]->state, this->data[ii]->mutationCounter);
			/*
			if (dt->getX() == 0 && dt->getY() == 0)
			{
				std::cout << "DATA MULT " << dt->getX() << " " << dt->getY() << " " << ii << std::endl;
				system("Pause");
			}*/
			//	dt->setState(sData[ii]->getState());
			*txx = *dt;
			txx++;
			delete dt;

		}
		return dat;
	}
	void deserialize(char* data, int buffLen)
	{
		uint32_t* tt2 = (uint32_t*)data;
		this->packetSize = *tt2;
		tt2++;
		this->iteration = *tt2;
		tt2++;
		cellDataStruct* ttb = (cellDataStruct*)tt2;

		for (int kk = 0; kk < this->packetSize; ++kk)
		{

			cellDataStruct* dt = new cellDataStruct(ttb->getX(), ttb->getY(), ttb->lifeTime, ttb->state, ttb->mutationCounter);
			/*
			if (dt->getX() == 0 && dt->getY() == 0)
			{
				std::cout << "DATA MULT2 " << dt->getX() << " " << dt->getY() << " " << kk << std::endl;
				system("Pause");
			}*/
			//dt->setState(ttb->getState());
			this->data.push_back(dt);
			//cout << dt->getX() << " TTT " << dt->getY() << endl;
			//	system("Pause");
			ttb++;
		}
	}
};


int GetNOProc();
bool ConnectTo(SOCKET &servSocket, char* IpAddress, int port);
bool SetupReceiveSocket(SOCKET &clientSocket, int port);
bool JoinWorkerPool(SOCKET& sock);
#endif