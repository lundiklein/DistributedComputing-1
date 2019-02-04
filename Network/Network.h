#pragma once
#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#include "stdafx.h"

#define WAIT_INTERV 1000

#define DATA_PACKET_TRANSFER_START 10
#define DATA_PACKET_TRANSFER_CONFIRMATION 11
#define DATA_PACKET_TRANSFER 12
#define DATA_PACKET_TRANSFER_FINISH 13
#define DATA_PACKET_TRANSFER_ABORT 14

//DLL_EXPORT struct dataPacketHeader;
#define MAX_PACKET_DATA_SIZE 512
DLL_EXPORT struct dataPacketHeader
{
	uint8_t action;
	uint32_t packetId;
	uint8_t isDataPacket;
	uint16_t dataSize;
	uint32_t fullDataSize;

	void deserialize(char* data)
	{
		dataPacketHeader* transfer = (dataPacketHeader*)data;
		*this = *transfer;
	};
	dataPacketHeader operator=(dataPacketHeader &data)
	{
		this->action = data.action;
		this->packetId = data.packetId;
		this->isDataPacket = data.isDataPacket;
		this->dataSize = data.dataSize;
		this->fullDataSize = data.fullDataSize;
		return *this;
	}


};
//TMP TO BE REMOVED
DLL_EXPORT struct isServerRequest
{
	uint8_t actionId;
	uint8_t response;
};
//------------------------
DLL_EXPORT struct networkDataPacket
{
	dataPacketHeader header;
	char* data = nullptr;
	networkDataPacket(uint8_t action, uint32_t packetId, uint16_t dataSize, uint32_t fullDataSize, uint8_t isDataPacket, char* dat)
	{
		this->header.action = action;
		this->header.packetId = packetId;
		this->header.isDataPacket = isDataPacket;
		this->header.dataSize = dataSize;
		this->header.fullDataSize = fullDataSize;
		if (this->header.isDataPacket == 1)
		{

			data = new char[dataSize];
			for (int ii = 0; ii < dataSize; ++ii)
			{
				*data = *dat;
				data++;
				dat++;
			}
		}
		else
		{
			data = nullptr;
		}
		
	}
	networkDataPacket(const networkDataPacket &ndp)
	{
		this->header.action = ndp.header.action;
		this->header.packetId = ndp.header.packetId;
		this->header.isDataPacket = ndp.header.isDataPacket;
		this->header.dataSize = ndp.header.dataSize;
		this->header.fullDataSize = ndp.header.fullDataSize;
		if (this->header.isDataPacket == 1)
		{

			data = new char[this->header.dataSize];
			for (int ii = 0; ii < this->header.dataSize; ++ii)
			{
				data[ii] = ndp.data[ii];
			}
		}
		else
		{
			data = nullptr;
		}
	}
	~networkDataPacket()
	{
		if(header.isDataPacket == 1 && header.dataSize > 0)
			delete[] data;
	}
	//CHECK
	networkDataPacket operator=(networkDataPacket &data)
	{
		this->header = data.header;
		if ((int)this->header.isDataPacket == 1 && this->header.dataSize > 0)
		{
			//std::cout << " = op " << (int) this->header.isDataPacket << std::endl;
			if (this->data != nullptr)
				delete[] this->data;
			this->data = new char[this->header.dataSize];
			for (int ii = 0; ii < header.dataSize; ++ii)
			{
				this->data[ii] = data.data[ii];
			}
		}
	
		return *this;
	}
	void setNetworkDataPacket(uint8_t action, uint32_t packetId, uint16_t dataSize, uint32_t fullDataSize, uint8_t isDataPacket, char* dat)
	{
		this->header.action = action;
		this->header.packetId = packetId;
		this->header.isDataPacket = isDataPacket;
		this->header.dataSize = dataSize;
		this->header.fullDataSize = fullDataSize;
		if (this->header.isDataPacket == 1)
		{
			if (data != nullptr)
				delete[] data;
			data = new char[dataSize];
			for (int ii = 0; ii < dataSize; ++ii)
			{
				data[ii] = *dat;
				dat++;
			}
		}
	}
	void setNetworkDataPacket(dataPacketHeader* header, char* dat)
	{
		this->header = *header;
		if (this->header.isDataPacket == 1)
		{
			if (data != nullptr)
				delete[] data;
			data = new char[this->header.dataSize];
			for (int ii = 0; ii < this->header.dataSize; ++ii)
			{
				data[ii] = *dat;
				dat++;
			}
		}
	}

	void serialize(char* packet)
	{
		ZeroMemory(packet, getBufferLength());


		dataPacketHeader* transfer = (dataPacketHeader*)packet;
		*transfer = this->header;
		transfer++;
		
		if (this->header.isDataPacket == 1)
		{
			char* trf = (char*)transfer;
			transfer--;
			for (int ii = 0; ii < this->header.dataSize; ++ii)
			{
				*trf = data[ii];
				trf++;
			}
		}

	}
	char* serialize()
	{
		char* packet = new char[getBufferLength()];
		ZeroMemory(packet, getBufferLength());
		dataPacketHeader* transfer = (dataPacketHeader*)packet;
		*transfer = this->header;
		transfer++;

		if (this->header.isDataPacket == 1)
		{
			char* trf = (char*)transfer;
			for (int ii = 0; ii < this->header.dataSize; ++ii)
			{
				*trf = data[ii];
				trf++;
			}
		}
		return packet;
	};
	void deserialize(char* data)
	{
		dataPacketHeader* transfer = (dataPacketHeader*)data;
		transfer++;
		char* trf = (char*)transfer;
		transfer--;
		//transfer->data = new char[transfer->header.dataSize];
		this->setNetworkDataPacket(transfer, trf);
		//*this = *transfer;

	//	std::cout << "DESER " << (int)this->header.dataSize << "/" << (int)transfer->dataSize << std::endl;
	};
	int getBufferLength()
	{
		return header.isDataPacket == 1 ? sizeof(dataPacketHeader) + header.dataSize * sizeof(char) : getMinBufferLength();
	}
	int getMaxBufferLength()
	{
		return sizeof(dataPacketHeader) + MAX_PACKET_DATA_SIZE * sizeof(char);// +sizeof(char*);
	}
	int getMinBufferLength()
	{
		return sizeof(dataPacketHeader);// +sizeof(char*);
	}
	void setData(char* data, int dataLen)
	{
		this->data = new char[dataLen];
		for (int ii = 0; ii < dataLen; ++ii)
		{
			this->data[ii] = data[ii];
		}
	}

};


DLL_EXPORT bool SetupReceiveSocket(SOCKET &sock, int port);
DLL_EXPORT bool ConnectTo(SOCKET &sock, char* IpAddress, int port);


DLL_EXPORT int GetNOProc();


DLL_EXPORT bool SetupUDPSocket(SOCKET &clientSocket, int port);

DLL_EXPORT int TransferMessage(SOCKET sock, char* data, int buffLen, int flags, bool isSending);

DLL_EXPORT bool Send(SOCKET sock, char* data, int buffLen, int flags, bool shouldConfirm);
DLL_EXPORT int Recv(SOCKET sock, char* &data, int flags, bool shouldConfirm, int dataBufferLen = 0);

//send(wPoolData->wPool->at(jj)->sock, data, buffLen, NULL
#endif