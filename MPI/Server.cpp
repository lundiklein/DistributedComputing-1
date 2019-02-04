#include "stdafx.h"
#include "Server.h"	

#define LOGIN_SERVER_PORT 6635
using namespace std;
DWORD WINAPI WorkerPoolThread(LPVOID param);
Server::Server()
{

}
Server::~Server()
{

}
bool Server::Initialize()
{
	ls_SHUTDOWN = CreateEvent(NULL, TRUE, FALSE, NULL);
	WSAData wsadat;
	if (WSAStartup(MAKEWORD(2, 2), &wsadat) != 0)
	{
		printf("WSASTARTUP ERROR: %d\n", WSAGetLastError());
		return false;
	}
	else
	{
		printf("WSASTARTUP SUCCESS\n");
	}

	return true;

}
bool Server::StartServer()
{
#pragma region ServerInitialization
	std::cout << "STARTING SERVER\n";
	if (!Initialize())
	{
		return false;
	}
	if (!SetupReceiveSocket(clientSocket[0], LOGIN_SERVER_PORT))
	{
		return false;
	}
	if ((ls_ACCEPT[0] = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("WSACREATE EVENT ERROR: %d\n", WSAGetLastError());
		return false;
	}
	if (WSAEventSelect(clientSocket[0], ls_ACCEPT[0], FD_ACCEPT) == SOCKET_ERROR)
	{
		printf("WSA SELECT ERROR: %d\n", WSAGetLastError());
		WSACloseEvent(ls_ACCEPT[0]);
		return false;
	}
	if (!SetupReceiveSocket(clientSocket[1], 6633))
	{
		return false;
	}
	if ((ls_ACCEPT[1] = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("WSACREATE EVENT ERROR: %d\n", WSAGetLastError());
		return false;
	}
	if (WSAEventSelect(clientSocket[1], ls_ACCEPT[1], FD_ACCEPT) == SOCKET_ERROR)
	{
		printf("WSA SELECT ERROR: %d\n", WSAGetLastError());
		WSACloseEvent(ls_ACCEPT[1]);
		return false;
	}
#pragma endregion
	DWORD ids;
	if (CreateThread(0, 0, ClientWorkerThread, (void*)this, 1, &ids) == NULL)
	{
		printf("CLIENT THREAD CRT ERROR: %d\n", GetLastError());
	}
	WSANETWORKEVENTS netEvent;
	while (WAIT_OBJECT_0 != WaitForSingleObject(ls_SHUTDOWN, 0))
	{
#pragma region HANDLE ACCEPT
		if (WSA_WAIT_TIMEOUT != WSAWaitForMultipleEvents(2, ls_ACCEPT, FALSE, 50, FALSE))
		{
			for (int ii = 0; ii < 2; ++ii)
			{
				if (WSAEnumNetworkEvents(clientSocket[ii], ls_ACCEPT[ii], &netEvent) == SOCKET_ERROR)
				{
					printf("EVENT ERROR %d\n", WSAGetLastError());
				}
				else
				{
					if ((netEvent.lNetworkEvents &FD_ACCEPT) && (netEvent.iErrorCode[FD_ACCEPT_BIT] == 0))
					{
						AcceptClient(ii);
					}
				}
			}
		}
#pragma endregion
	}


	return true;

}
uint32_t Server::GetFirstAvaiableId()
{
	if (clientInfoVect.empty()) return 1;
	uint32_t size = 1;
	for (int ii = 0; ii < clientInfoVect.size(); ++ii)
	{
		if ((size) != clientInfoVect[ii]->id) return size;
		size++;
	}
	return size;
}
uint32_t Server::GetFirstAvaiableServerId()
{
	if (workerPoolVector.empty()) return 1;
	uint32_t size = 1;
	for (int ii = 0; ii < workerPoolVector.size(); ++ii)
	{
		if ((size) != workerPoolVector[ii]->serverId) return size;
		size++;
	}
	return size;
}
bool Server::SISOCC(SOCKET sock)
{
	cout << "SISOCC" << endl;
	//char data[sizeof(isServerRequest)];
	char* data = new char[sizeof(isServerRequest)];
	isServerRequest* rq = (isServerRequest*)data;
	rq->actionId = 0;
	rq->response = 0;
	if (Send(sock, data, sizeof(data), 0, true))
	{
		cout << "SISOCC SEND" << endl;

		ZeroMemory(data, sizeof(data));
		Recv(sock, data, 0, true);

		isServerRequest* rq2 = (isServerRequest*)data;
		cout << "SISOCC RECV " << (int)rq2->actionId << " " << (int)rq2->response << endl;
		if (rq2->actionId == 0 && rq2->response == 1)
		{
			cout << "SISOCC SERVER" << endl;
			delete[] data;
			return true;
		}
	}
	else
	{
		cout << "SISOCC SEND ERROR " << WSAGetLastError() << endl;
	}
	delete[] data;
	return false;
}
void Server::handleWorkerAccept(SOCKET workerSocket, SOCKADDR_IN clientAddr)
{
	cout << "HWA" << endl;
	//char data[sizeof(joinWorkerPoolRequest)];
	char* data = new char[sizeof(joinWorkerPoolRequest)];
	if (Recv(workerSocket, data, 0, true) > 0)
	{
		joinWorkerPoolRequest* rq = (joinWorkerPoolRequest*)data;
		workerData* wData = new workerData;
		wData->makeWorkerDataFromRQ(rq);
		wData->serverId = GetFirstAvaiableServerId();
		cout << "Worker id " << wData->serverId << endl;
		wData->sock = workerSocket;
		wData->serverAddr = clientAddr;
		wData->minPriority = rq->minPriority;
		wData->threads = rq->threads;
		wData->ram = rq->ram;
		wData->status = STATUS_FREE;
		cout << "Worker id " << wData->serverId << " THREADS / RAM " << (int)wData->threads << "/" << wData->ram << endl;

		workerPoolVector.push_back(wData);
		workerPoolSocketVector.push_back(workerSocket);
	}
	delete[] data;
	

}
void Server::handleClientAccept(SOCKET workerSocket, SOCKADDR_IN clientAddr)
{
	cout << "ADD CLIENT" << endl;
	ClientInf* clit = new ClientInf;
	clit->socket = workerSocket;
	clit->addr = clientAddr;
	clit->id = GetFirstAvaiableId();
	strcpy(clit->login, "");
	socketVector.push_back(workerSocket);
	WSAEVENT socketEvent = WSACreateEvent();
	WSAEventSelect(workerSocket, socketEvent, FD_READ);
	eventVector.push_back(socketEvent);
	clientInfoVect.push_back(clit);


}
void Server::AcceptClient(int ii)
{
	SOCKADDR_IN clientAddr;
	int clInfoLen = sizeof(clientAddr);
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ((sock = accept(clientSocket[ii], (SOCKADDR*)&clientAddr, &clInfoLen)) == SOCKET_ERROR)
	{
		printf("ACCEPT CLIENT ERROR: %d\n", WSAGetLastError());
		return;
	}
	else
	{
		printf("CLIENT CONNECTED IP: %s PORT: %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
	}
	//handle server connection
	if (SISOCC(sock))
	{
		handleWorkerAccept(sock, clientAddr);
	}
	else
	{
		handleClientAccept(sock, clientAddr);
	}
	/*
		TO ADD -> SEND RESULT RESPONSE TO CLIENT/WORKER
	*/
	cout << "ACCEPT FINISH" << endl;

}



bool SRR(SOCKET sock, bool result)
{
	char data[1];
	result ? strcpy(data, "1") : strcpy(data, "0");
	if (!Send(sock, data, sizeof(data), NULL, true))
	{
		printf("SEND DATA TO CLIENT ERROR: %d (ID: %d)\n ", WSAGetLastError(), (int)result);
		cout << "SRR SENT FALSE " << data << endl;
		return false;
	}
	cout << "SRR SENT TRUE " << data << endl;
	return true;
}
bool SLR(SOCKET sock, uint32_t netId)
{
	char data[sizeof(baseRequest)];
	baseRequest* rq = (baseRequest*)data;
	rq->actionId = LOGINRQ;
	rq->NetId = netId;
	if (!Send(sock, data, sizeof(data), NULL, true))
	{
		printf("SEND DATA TO CLIENT ERROR: %d\n ", WSAGetLastError());
		return false;
	}
	return true;
}




std::vector<workerData*>* assignWorkerPool(Server* serv)
{
	std::vector<workerData*>* wPool = new std::vector<workerData*>();
	for (int ii = 0; ii < serv->workerPoolVector.size(); ++ii)
	{
		if (serv->workerPoolVector[ii]->status == STATUS_FREE)
		{
			serv->workerPoolVector[ii]->status = STATUS_WORKING;
			wPool->push_back(serv->workerPoolVector[ii]);
		}
	}
	return wPool;
}

DWORD WINAPI ClientWorkerThread(LPVOID param)
{
	WSANETWORKEVENTS netEvent;
	Server* serv = (Server*)param;
	char* data = nullptr;// [1024];
#pragma region HANDLE RECV
	while (WAIT_OBJECT_0 != WaitForSingleObject(ls_SHUTDOWN, 0))
	{
		if (serv->socketVector.empty())
		{
			Sleep(5);
			continue;
		}
		for (int ii = 0; ii < serv->socketVector.size(); ++ii)
		{
			if (serv->socketVector[ii] == INVALID_SOCKET)
			{
				cout << "INVALID SOCKET " << ii << endl;
				continue;
			}
			DWORD Index = WSAWaitForMultipleEvents(1, &serv->eventVector[ii], FALSE, 5, FALSE);
			if ((Index != WSA_WAIT_FAILED) && (Index != WSA_WAIT_TIMEOUT))
			{
				if (WSAEnumNetworkEvents(serv->socketVector[ii], serv->eventVector[ii], &netEvent) != SOCKET_ERROR)
				{
					if ((netEvent.lNetworkEvents &FD_READ) && (netEvent.iErrorCode[FD_READ_BIT] == 0))
					{
						if ((Recv(serv->socketVector[ii], data, 0, true)))
						{
							//handle data login, getHosts, logout
							baseRequest* req = (baseRequest*)data;
							switch (req->actionId)
							{

							case CALCRQ:
							{
								/*
									1) assign worker pool
									2) create new thread to handle calculation / dataTransfer
									{					--- new thread ---
										1) handle data initialization and data partitioning
										2) handle data transfer -> client <-> server <-> workerPool 
										3) free workerPool on finish
									}
								*/
								calculateRequest* cdat = (calculateRequest*)data;
								workerPoolData* wData = new workerPoolData;
								wData->wPool = assignWorkerPool(serv);
								wData->iterations = cdat->iterations;
								wData->cellNumber = cdat->cellNumber;
								/*
									ADD PROBLEM INIT DATA TO wData
								*/
								DWORD ids;
								if (CreateThread(0, 0, WorkerPoolThread, (void*)wData, 1, &ids) == NULL)
								{
									printf("WORKER THREAD CRT ERROR: %d\n", GetLastError());
								}
								break;
							}
#pragma region LOGINRQ
							case LOGINRQ:
							{
								cout << " LOGIN RQ " << endl;
								loginRequest* ldat = (loginRequest*)data;
								uint32_t netId = 0;
								if (/*login*/(ldat->login, ldat->password))
								{
									//check if is logged in
									for (int jj = 0; jj < serv->clientInfoVect.size(); ++jj)
									{
										if (strcmp(serv->clientInfoVect[jj]->login, ldat->login) == 0)
										{
											cout << "Player " << ldat->login << " DC - New player on this acc" << endl;
											WSACloseEvent(serv->eventVector[jj]);
											closesocket(serv->socketVector[jj]);
											serv->eventVector.erase(serv->eventVector.begin() + jj);
											serv->socketVector.erase(serv->socketVector.begin() + jj);
											//Send info to server to dc player
											if (jj < ii)
											{
												ii--;
											}
										}
									}
									netId = serv->clientInfoVect[ii]->id;
									//strcpy(Server->clientInfoVect[ii]->login, ldat->login);
									SLR(serv->socketVector[ii], netId);
									cout << "PLAYER LOGGED IN " << ldat->login << endl;
								}
								else
								{
									SLR(serv->socketVector[ii], netId);
									WSACloseEvent(serv->eventVector[ii]);
									closesocket(serv->socketVector[ii]);
									serv->eventVector.erase(serv->eventVector.begin() + ii);
									serv->socketVector.erase(serv->socketVector.begin() + ii);
									ii--;
									cout << "PLAYER PASS OR LOGIN ERROR " << ldat->login << endl;
								}

								//login(ldat->login, ldat->password) ? SRR(Server->socketVector[ii], true) : SRR(Server->socketVector[ii], false);
								break;
							}
#pragma endregion
#pragma region LOGOUTRQ
							case LOGOUTRQ:
							{
								cout << "LOGOUT" << endl;

								WSACloseEvent(serv->eventVector[ii]);
								closesocket(serv->socketVector[ii]);
								serv->eventVector.erase(serv->eventVector.begin() + ii);
								serv->socketVector.erase(serv->socketVector.begin() + ii);
								serv->clientInfoVect.erase(serv->clientInfoVect.begin() + ii);
								ii--;
								break;
							}
#pragma endregion
				

							default:
								break;
							}
						}
					}
					else
					{

					}
				}
			}
		}
	}
#pragma endregion
	return 0;
}


int getPartitionBorderIndex(std::vector<cellDataStruct*> dataSet, cellDataStruct* dataBorder)
{
	for (int ii = 0; ii < dataSet.size(); ++ii)
	{
		if (dataSet[ii]->getX() == dataBorder->getX() && dataSet[ii]->getY() == dataBorder->getY())
			return ii;
	}
	return -1;
}
std::vector<cellDataStruct*> splitData(std::vector<cellDataStruct*> dataSet, cellDataStruct* dataBorder, double relativeSize)
{
	std::vector<cellDataStruct*> partitionedData;// = std::vector<cellDataStruct*>();
	int dataSetLen = dataSet.size();
	int dataBorderIndex = getPartitionBorderIndex(dataSet, dataBorder);
	if (dataBorderIndex == 0)
		dataBorderIndex--;
		//	partitionedData.push_back(dataBorder);
	
	for (int ii = dataBorderIndex + 1; ii < dataBorderIndex + 1 + relativeSize && ii < dataSet.size(); ++ii)
	{
		partitionedData.push_back(dataSet[ii]);
	}
	int lastIndex = dataBorderIndex + 1 + relativeSize;

	dataBorder = lastIndex < dataSet.size() ? dataSet.at(lastIndex) : dataSet.at(0);
	return partitionedData;
}
int updateDataSet(std::vector<cellDataStruct*> dataSet, cellDataStruct* newData, int lastUpdatedIndex)
{
	for (int ii = lastUpdatedIndex; ii < dataSet.size(); ++ii)
	{
		if (newData->getX() == dataSet.at(ii)->getX() && newData->getY() == dataSet.at(ii)->getY())
		{
			*dataSet.at(ii) = *newData;
			return ii;
		}
	}
	if (lastUpdatedIndex > 0)
	{
		for (int ii = 0; ii < lastUpdatedIndex; ++ii)
		{
			if (newData->getX() == dataSet.at(ii)->getX() && newData->getY() == dataSet.at(ii)->getY())
			{
				*dataSet.at(ii) = *newData;
				return ii;
			}
		}
	}
	return 0;
}
std::vector<cellDataStruct*> initializeDataSet(workerPoolData* data)
{
	std::vector<cellDataStruct*> dataSet;// = std::vector<cellDataStruct*>();
	int gridSize = sqrt(data->cellNumber);
	int x = 0, y = 0;
	for (int ii = 0; ii < data->cellNumber; ++ii)
	{
		/*
			SETUP GRID -> FIND CELL X,Y BASED ON CELL NUMBER -> 
		*/
		dataSet.push_back(new cellDataStruct(x, y, 0, 0, 0));
		if (x >= gridSize)
		{
			++y;
			x = 0;
		}
		++x;
	}
	return dataSet;
}

DWORD WINAPI WorkerPoolThread(LPVOID param)
{

	LARGE_INTEGER prevTime, currTime, processorTickFrequency;
	QueryPerformanceFrequency(&processorTickFrequency);
	QueryPerformanceCounter(&prevTime);

	WSANETWORKEVENTS netEvent;
	workerPoolData* wPoolData = (workerPoolData*)param;
	int workerPoolThreadNumber = wPoolData->getWorkerPoolThreadNO();
	
	std::vector<WSAEVENT> workerPoolEventVector = std::vector<WSAEVENT>();
	for (int ii = 0; ii < wPoolData->wPool->size(); ++ii)
	{
		WSAEVENT socketEvent = WSACreateEvent();
		WSAEventSelect(wPoolData->wPool->at(ii)->sock, socketEvent, FD_READ);
		workerPoolEventVector.push_back(socketEvent);
	}
	double dataPart = wPoolData->cellNumber / (double)workerPoolThreadNumber;
	int dataSplitCounter = 0;
	for (int jj = 0; jj < wPoolData->wPool->size(); ++jj)
	{

		uint32_t dtp = dataPart * wPoolData->wPool->at(jj)->threads;
		if (dtp < dataPart * wPoolData->wPool->at(jj)->threads)
		{
			dtp++;
		}

		int gridSize = sqrt(wPoolData->cellNumber);
		int x = 0, y = 0, currX = 0, currY = 0;
		for (int ii = dataSplitCounter; ii < wPoolData->cellNumber; ++ii)
		{
			if (x >= gridSize)
			{
				++y;
				x = 0;
			}
			++x;
			if (dtp == ii)
			{
				currX = x;
				currY = y;
				dataSplitCounter = ii + 1;
				break;
			}
		}
		//----------------------------------------------------------------------------------------
		ndataPacket* packet = new ndataPacket(dtp, wPoolData->iterations, currX, currY, gridSize);
		char* data = packet->serialize();
		while (!Send(wPoolData->wPool->at(jj)->sock, data, sizeof(ndataPacket), NULL, true))
		{
			printf("Send error xx %d\n", WSAGetLastError());
			delete[] data;
			data = nullptr;
		}
		delete[] data;
		data = nullptr;
		//----------------------------------------------------------------------------------------
	}
	std::vector<int> finishedNodes;
	std::vector<ndataPacket*> allData;
	std::vector<transferHistory*> history;
	while (finishedNodes.size() < wPoolData->wPool->size())
	{
	//	cout << " LOOP " << endl;
		for (int jj = 0; jj < wPoolData->wPool->size(); ++jj)
		{
			bool skip = false;
			for (std::vector<int>::iterator iter = finishedNodes.begin(); iter != finishedNodes.end(); iter++)
			{
				if (*iter == jj)
				{
					skip = true;
					break;
				}
			}
			if (skip)
			{

			//	cout << " SKIP " << endl;
				continue;
			}
			char* data = nullptr;
			int dataSize = Recv(wPoolData->wPool->at(jj)->sock, data, 0, true);
			if (dataSize > 0)
			{
				//cout << "RECEIVED DATA FROM " << jj << endl;
				ndataPacket* packet = new ndataPacket(0, 0, 0, 0, 0);
				packet->deserialize(data);
				if (packet->citeration == wPoolData->iterations)
				{
					finishedNodes.push_back(jj);
				}
				allData.push_back(packet);
				history.push_back(new transferHistory(jj, *packet));
			//	std::cout << "zdrowe: " << packet->healthy << " zmutowane: " << packet->mutated << " rakowe: " <<
			//		packet->canceroues << " martwe: " << packet->dead << " t =  " << packet->citeration << std::endl;
			}
			if (data != nullptr)
			{
				delete[] data;
				data = nullptr;
			}

		}	
	}
	for (int ii = 0; ii < wPoolData->wPool->size(); ++ii)
	{
		for (int jj = 1; jj < wPoolData->iterations; ++jj)
		{
			bool received = false;
			for (int kk = 0; kk < history.size(); ++kk)
			{
				if (history[kk]->stationId == ii && history[kk]->packet.citeration == jj)
				{
					received = true;
				}
			}
			//check if all data has been transfered
			if (!received)
			{
				cout << "FIXING MISSING DATA t = " << jj << endl;
				ndataPacket* packet = new ndataPacket(0, 0, 0, 0, 0);
				packet->citeration = jj;
				char* data = packet->serialize();
				Send(wPoolData->wPool->at(ii)->sock, data, sizeof(ndataPacket), NULL, true);
				delete packet;
				delete[] data;
				data = nullptr;
				bool waiting = true;
				while (waiting)
				{
					int dataSize = Recv(wPoolData->wPool->at(ii)->sock, data, 0, true);
					if (dataSize > 0)
					{
						ndataPacket* packet = new ndataPacket(0, 0, 0, 0, 0);
						packet->deserialize(data);
						allData.push_back(packet);
						history.push_back(new transferHistory(ii, *packet));
					}
				}
			}
		}
		//send info that we are done
		ndataPacket* packet = new ndataPacket(0, 0, 0, 0, 0);
		packet->citeration = 0;
		char* data = packet->serialize();
		Send(wPoolData->wPool->at(ii)->sock, data, sizeof(ndataPacket), NULL, true);
	}
	std::vector<ndataPacket*> allDataCalculated;
	std::ofstream dataOutput;
	dataOutput.open("data.txt");
	for (int ii = 0; ii < wPoolData->iterations; ++ii)
	{
		if (ii == 0)
		{

			ndataPacket* data = new ndataPacket(wPoolData->cellNumber, wPoolData->iterations, 0, 0, 0);
			data->healthy = wPoolData->cellNumber;
			allDataCalculated.push_back(data);
			std::cout << "zdrowe: " << data->healthy << " zmutowane: " << data->mutated << " rakowe: " <<
				data->canceroues << " martwe: " << data->dead << " t =  " << data->citeration << std::endl;
			dataOutput << data->healthy << " " << data->mutated << "  " << data->canceroues << "  " << data->dead
				<< " " << data->citeration << std::endl;
		}
		else
		{
			ndataPacket* data = new ndataPacket(0, wPoolData->iterations, 0, 0, 0);
			for (int jj = 0; jj < allData.size(); ++jj)
			{
				if (allData[jj]->citeration == ii)
				{
					data->citeration = allData[jj]->citeration;
					data->cellNumber += allData[jj]->cellNumber;
					data->healthy += allData[jj]->healthy;
					data->mutated += allData[jj]->mutated;
					data->canceroues += allData[jj]->canceroues;
					data->dead += allData[jj]->dead;
				}
			}
			allDataCalculated.push_back(data);
			std::cout << "zdrowe: " << data->healthy << " zmutowane: " << data->mutated << " rakowe: " <<
				data->canceroues << " martwe: " << data->dead << " t =  " << data->citeration << std::endl;

			dataOutput << data->healthy << " " << data->mutated << "  " << data->canceroues << "  " << data->dead
				<< " " << data->citeration << std::endl;
		}
		
		

	}
	for (std::vector< ndataPacket* >::iterator it = allData.begin(); it != allData.end(); ++it)
	{
		delete (*it);
	}
	allData.clear();


	for (std::vector< ndataPacket* >::iterator it = allDataCalculated.begin(); it != allDataCalculated.end(); ++it)
	{
		delete (*it);
	}
	allDataCalculated.clear();

	QueryPerformanceCounter(&currTime);
	double ping = calculateTime(currTime, prevTime, processorTickFrequency);
	cout << "Time spend on symulation " << ping/1000. <<"[s]" << endl;
}

DWORD WINAPI WorkerPoolThreadD(LPVOID param)
{
	WSANETWORKEVENTS netEvent;
	workerPoolData* wPoolData = (workerPoolData*)param;
	int workerPoolThreadNumber = wPoolData->getWorkerPoolThreadNO();

	std::vector<WSAEVENT> workerPoolEventVector = std::vector<WSAEVENT>();
	for (int ii = 0; ii < wPoolData->wPool->size(); ++ii)
	{
		WSAEVENT socketEvent = WSACreateEvent();
		WSAEventSelect(wPoolData->wPool->at(ii)->sock, socketEvent, FD_READ);
		workerPoolEventVector.push_back(socketEvent);
	}
	//DATA SET INITIALIZATION -> SETUP CELL GRID 
	std::vector<cellDataStruct*> dataSet = initializeDataSet(wPoolData);
	//SIMULATION
	for (int ii = 0; ii < wPoolData->iterations; ++ii)
	{
		loopstart:
		//TESTING
		//=======================
		//SEND TIME STEP DATA TO WORKERS
		//DATA PARTITION FIX -> INT ROUND DOWN
		double dataPart = wPoolData->cellNumber / (double)workerPoolThreadNumber;
		if (dataPart * workerPoolThreadNumber != wPoolData->cellNumber)
		{
			cout << "DATA PARTITION ERROR" << endl;
		}
		cout << "Time = " << ii << " ";
		dataSet[0]->printInto();
	//	cout << " DATA PART " << dataPart << endl;
		//system("Pause");
		cellDataStruct* dataBorder = new cellDataStruct(0, 0, 0, 0, 0);
		for (int jj = 0; jj < wPoolData->wPool->size(); ++jj)
		{

			int dtp = dataPart * wPoolData->wPool->at(jj)->threads;
			if (dtp < dataPart * wPoolData->wPool->at(jj)->threads)
			{
				dtp++;
			}
			std::vector<cellDataStruct*> sData = splitData(dataSet, dataBorder, dtp);
			*dataBorder = *sData.at(sData.size() - 1);
			dataPacket pData = dataPacket();
			int buffLen = sizeof(2 * sizeof(uint32_t)) + sData.size() * sizeof(cellDataStruct);
			cout << "BUFFL " << buffLen << endl;
			//----------------------------------------------------------------------------------------
			//char* data = new char[buffLen];
			//ZeroMemory(data, buffLen);
			char* data = pData.serialize(buffLen, sData, ii);
			//Data transfer
			//----------------------------------------------------------------------------------------
			if (!Send(wPoolData->wPool->at(jj)->sock, data, buffLen, NULL, true))
			{
				printf("Send error xx %d\n", WSAGetLastError());
				delete[] data;
				sData.clear();
			//	system("Pause");
				goto loopstart;
			}
			delete[] data;
			sData.clear();
			//----------------------------------------------------------------------------------------
			wPoolData->wPool->at(jj)->lastPacketSize = buffLen;
			//system("Pause");
		}
		delete dataBorder;
		std::vector<cellDataStruct*> tempCellDataVector;// = std::vector<cellDataStruct*>();
		char* data = nullptr;// new char[wPoolData->wPool->at(jj)->lastPacketSize];
							 //ZeroMemory(data, sizeof(data));
		std::vector<int> recvStationId;
		while (recvStationId.size() < wPoolData->wPool->size())
		{

			for (int jj = 0; jj < wPoolData->wPool->size(); ++jj)
			{
				bool skip = false;
				for (std::vector<int>::iterator iter = recvStationId.begin(); iter != recvStationId.end(); iter++)
				{
					if (*iter == jj)
					{
						skip = true;
						break;
					}
				}
				if (skip)
					continue;
				int dataSize = Recv(wPoolData->wPool->at(jj)->sock, data, 0, true);
				if (dataSize > 0)
				{

					recvStationId.push_back(jj);
					cout << "RECEIVED DATA FROM " << jj << endl;

					dataPacket* test2 = new dataPacket();
					if (!test2->deserialize(data, wPoolData->wPool->at(jj)->lastPacketSize, ii))
					{

						for (int xx = 0; xx < test2->data.size(); xx++)
						{

							//if (test2->data[xx]->getX() == 0 && test2->data[xx]->getY() == 0)
							//{
							//	std::cout << "DATA MULT " << test2->data[xx]->getX() << " " << test2->data[xx]->getY() << " " << xx << endl;
							//	system("Pause");
							//}
							//std::cout << xx << std::endl;
							cellDataStruct* dat = new cellDataStruct(*test2->data[xx]);
							tempCellDataVector.push_back(dat);

						}

						//zdrowa/1zmutowana/2nowo/-1|255/martwa
					}
					delete test2;


				}
				delete[] data;
				data = nullptr;
			}
		}
		/*
		DWORD Index = WSAWaitForMultipleEvents(workerPoolEventVector.size(), &workerPoolEventVector[0], TRUE, 60000, FALSE);
		bool dataError = false;
		while ((Index == WSA_WAIT_FAILED) || (Index == WSA_WAIT_TIMEOUT))
		{
			Sleep(100);
			Index = WSAWaitForMultipleEvents(workerPoolEventVector.size(), &workerPoolEventVector[0], TRUE, 60000, FALSE);
		}
		if ((Index != WSA_WAIT_FAILED) && (Index != WSA_WAIT_TIMEOUT))
		{
			for (int jj = 0; jj < wPoolData->wPool->size(); ++jj)
			{

				if (wPoolData->wPool->at(jj)->sock == INVALID_SOCKET)
				{
					cout << "INVALID SOCKET " << jj << endl;
					continue;
				}
				if (WSAEnumNetworkEvents(wPoolData->wPool->at(jj)->sock, workerPoolEventVector[jj], &netEvent) != SOCKET_ERROR)
				{
					
				
					
					
				}
				else
				{
					cout << "ERROR 1 " << WSAGetLastError() << endl;
				}
			}
		}
		*/
		int testCo = 0;
		end:
		if (tempCellDataVector.size() != dataSet.size())
		{
			cout << "Data vector sizes doesn't match " << tempCellDataVector.size() << "/" << dataSet.size()  << endl;
			ii--;
	
		}
		else
		{
			cellDataStruct* firstCell = new cellDataStruct;
			*firstCell = *tempCellDataVector[0];
			
		//	cout << "Data vector sizes " << tempCellDataVector.size() << "/" << dataSet.size() << endl;
			for (int jj = 0; jj < dataSet.size(); ++jj)
			{
			//	cout << firstCell->getX() << " " << firstCell->getY() << " / " << dataSet[jj]->getX() << " " << dataSet[jj]->getY() << endl;
			//	cout << "Data vector sizes " << tempCellDataVector.size() << "/" << dataSet.size() << "/" << jj << endl;
			//	system("Pause");
				if (firstCell->getX() == dataSet[jj]->getX() && firstCell->getY() == dataSet[jj]->getY())
				{
					for (int kk = 0; kk < tempCellDataVector.size(); ++kk)
					{
						//TMP DISABLED
						*dataSet[jj + kk] = *tempCellDataVector[kk];
						dataSet[jj + kk]->lifeTime++;
					}

					break;
				}
			}
			delete firstCell;
		}
		int zdrowe = 0, zmutowane = 0, rakowe = 0, martwe = 0;
		for (int jj = 0; jj < dataSet.size(); ++jj)
		{
			switch (dataSet[jj]->getState())
			{
			case 0:
				zdrowe++;
				break;
			case 1:
				zmutowane++;
				break;
			case 2:
				rakowe++;
				break;
			case -1:
			case 255:
				martwe++;
				break;
			}

		}

		for (std::vector< cellDataStruct* >::iterator it = tempCellDataVector.begin(); it != tempCellDataVector.end(); ++it)
		{
			delete (*it);
		}
		tempCellDataVector.clear();
		printf("STATS: ZDROWE - %d, ZMUTOWANE - %d, NOWOTWOROWE - %d, MARTWE - %d, T = %d\n TestCo = %d\n", zdrowe, zmutowane, rakowe, martwe, ii, testCo);
	//	system("Pause");
	}
	for (std::vector< cellDataStruct* >::iterator it = dataSet.begin(); it != dataSet.end(); ++it)
	{
		delete (*it);
	}
	dataSet.clear();
	

	return 0;
}


int main()
{
	Server Serv = Server();
	Serv.StartServer();
	system("PAUSE");
}