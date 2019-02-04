#include "stdafx.h"
#include "NetworkCustom.h"
#include "Simulation.h"
using namespace std;



int main(int argc, char** argv)
{

	std::vector<cellDataStruct*> cellData;
	int iterations = 0;
	int cellNumber = 0;
	srand(time(NULL));
	SOCKET sock = INVALID_SOCKET;
	cellDataStruct* allDataArray = nullptr;


	if (JoinWorkerPool(sock))
	{
		std::cout << "JWP S" << std::endl;
	}
	char* data = nullptr;// = new char[80008];
	int dataSize = Recv(sock, data, 0, true);
	while (dataSize <= 0)
	{
		Sleep(1);
		dataSize = Recv(sock, data, 0, true);
	}
	ndataPacket* packet = new ndataPacket(0, 0, 0, 0, 0);
	packet->deserialize(data);
	int startX = packet->startX;
	int startY = packet->startY;
	int gridSize = packet->gridSize;
	for (int ii = 0; ii < packet->cellNumber; ++ii)
	{
		cellDataStruct* cds = new cellDataStruct(startX, startY, 0, 0, 0);
		if (startX >= gridSize)
		{
			++startY;
			startX = 0;
		}
		++startX;
		cellData.push_back(cds);
	}
	iterations = packet->miteration;
	cellNumber = packet->cellNumber;

	allDataArray = new cellDataStruct[cellData.size()];
	for (int ii = 0; ii < cellData.size(); ++ii)
	{
		allDataArray[ii] = *cellData[ii];
	}

	
	std::vector<ndataPacket*> history;
	Simulation simulation = Simulation();
	for (int ii = 0; ii < iterations; ++ii)
	{
		//tutaj cellDataScater
		simulation.Simulate(cellData, 1.2f, ii, packet);
		cout << "SUM " << ii << " "  << packet->healthy << endl;
		ndataPacket* packetT = new ndataPacket(0, 0, 0, 0, 0);

		*packetT = *packet;
		history.push_back(packetT);
	}
	delete packet;
	/*
		dodaæ check od serwera czy wszystkie info dosz³o
	*/

	for (int ii = 0; ii < history.size(); ++ii)
	{
		Sleep(10);
		char* data = history[ii]->serialize();
		//	cout << "Send " << ii << " " << history[ii]->citeration << endl;
		if (!Send(sock, data, sizeof(ndataPacket), NULL, true))
		{
			//cout << "Send error " << WSAGetLastError() << " " << my_rank << endl;
			ii--;
		}
		if (data != nullptr)
		{
			delete[] data;
			data = nullptr;
		}
	}
	while (true)
	{
		char* data = nullptr;
		int dataSize = Recv(sock, data, 0, true);
		ndataPacket* packet = new ndataPacket(0, 0, 0, 0, 0);
		if (dataSize > 0)
		{
			packet->deserialize(data);
			if (packet->citeration == 0)
				break;
			else
			{
				for (int ii = 0; ii < history.size(); ++ii)
				{
					if (history[ii]->citeration == packet->citeration)
					{
						char* data = history[ii]->serialize();
						if (!Send(sock, data, sizeof(ndataPacket), NULL, true))
						{
							printf("Send error %d\n", WSAGetLastError());
						}
					}
				}
			}
		}
		delete packet;
	}
	for (std::vector< ndataPacket* >::iterator it = history.begin(); it != history.end(); ++it)
	{
		delete (*it);
	}
	history.clear();
	return 0;
}