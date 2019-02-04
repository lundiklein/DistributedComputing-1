#ifndef _LOGIN_SERVER_H_
#define _LOGIN_SERVER_H_

/************************TODO************************
*CLIENT LIST				-> MAKE IT
*CLIENT REMOVAL				-> FIX MEMORY LEAK
*SERVERS CLIENT LIST SHARE	-> CONNECTION + UPDATES
*CONSOLE COMMAND SYSTEM		-> CONTROL OVER SERVER
*REMOTE CONTROL SYSTEM
*STRESS TESTS
*CLEAN UP FUNCTION			-> EXIT CLEAN UP
*
*****************************************************/
#include "Network.h"

#define TIMEOUT 20000
DWORD WINAPI AcceptThread(LPVOID param);
DWORD WINAPI ClientWorkerThread(LPVOID param);

HANDLE ls_ACCEPT[2];
HANDLE ls_SHUTDOWN = NULL;
/*
client info
-id
-login
-addr
lobby info
-serverId
-serverName
-serverMap
-serverPlayer/MaxPlayers
-serverPassword
-serverPing
-serverIP/Port
//------------------------------------------------------------------
1) klient ³¹czy siê z serwerem - loguje siê
2) serwer sprawdza log/pass w db
3) jeœli jest ok to dodaje gracza do listy online i wysy³a mu serwery do lobby
4) klient wychodzi z gry - nastêpuje usuniêcie z listy graczy i zamkniêcie po³¹czenia
5) jeœli gracz wyjdzie z serwera wraca do lobby po czym nastêpuje odœwie¿enie lobby
//------------------------------------------------------------------
po³aczenie klient - lobby jest ca³y czas utrzymane -> udp? tcp?
-tworzymy socket akceptuj¹cy po³¹czenia
-onConnect tworzymy nowy socket i na nim przyjmujemy po³¹czenie
-odbieramy dane z nowostworzonego socketu w systemie wielow¹tkowym, zale¿nym od liczby pod³¹czonych klientów
//------------------------------------------------------------------
-na ka¿dego gracza przypada 1 socket i 1 event
*/
using namespace std;
#pragma region REQUEST STRUCTS
#define CALCRQ 66

#define LOGINRQ 1
#define LOGOUTRQ 4
#define HOSTSERVER 5
#define REGISTERRQ 7

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
	cellDataStruct operator=(const cellDataStruct &data)
	{
		this->x = data.x;
		this->y = data.y;
		this->lifeTime = data.lifeTime;
		this->state = data.state;
		this->mutationCounter = data.mutationCounter;
		return *this;
	}
	void printInto()
	{
		std::cout <<" X = " << this->x << " " <<
			" Y = " << this->y << " " <<
			" LT = " << " X = " << (int)this->lifeTime << " " <<
			" S = " << (int)this->state << " " <<
			" MC = " << (int)this->mutationCounter << std::endl;
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
	ndataPacket()
	{
		cellNumber = 0;
		citeration = 0;
		miteration = 0;
		healthy = 0;
		mutated = 0;
		canceroues = 0;
		dead = 0;
		startX = 0;
		startY = 0;
		gridSize = 0;
	}
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
struct transferHistory
{
	uint32_t stationId;
	ndataPacket packet;
	transferHistory(uint32_t stationId, ndataPacket packet)
	{
		this->stationId = stationId;
		this->packet = packet;
	}
};

struct dataPacket
{
	uint32_t packetSize;
	uint32_t iteration;

	/**/
	std::vector<cellDataStruct*> data;
	//std::vector<cellDataStruct*> dataVector;
	~dataPacket()
	{
		for (std::vector< cellDataStruct* >::iterator it = data.begin(); it != data.end(); ++it)
		{
			delete (*it);
		}
		data.clear();
	}
	int calculatePacketSize(std::vector<cellDataStruct*> dataVector)
	{
		return sizeof(2 * sizeof(uint32_t)) + dataVector.size() * sizeof(cellDataStruct);
	}
	char* serialize(int buffLen, std::vector<cellDataStruct*> sData, int ii)
	{
		char* dat = new char[calculatePacketSize(sData)];
		ZeroMemory(dat, calculatePacketSize(sData));
		dataPacket* tt = (dataPacket*)dat;
		uint32_t* tx = (uint32_t*)dat;
		*tx = sData.size();
		tx++;
		*tx = ii;
		tx++;
		cellDataStruct* txx = (cellDataStruct*)tx;
		for (int ii = 0; ii < sData.size(); ++ii)
		{
			cellDataStruct* dt = new cellDataStruct(sData[ii]->getX(), sData[ii]->getY(),
				sData[ii]->lifeTime, sData[ii]->state, sData[ii]->mutationCounter);
			/*
			if (dt->getX() == 0 && dt->getY() == 0 )
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

	bool deserialize(char* data, int buffLen, int iteration)
	{
		bool shouldSkip = false;
		uint32_t* tt2 = (uint32_t*)data;
		this->packetSize = *tt2;
		tt2++;
		this->iteration = *tt2;
		tt2++;
		cellDataStruct* ttb = (cellDataStruct*)tt2;

		if (sizeof(2 * sizeof(uint32_t)) + this->packetSize * sizeof(cellDataStruct) != buffLen)
		{
			cout << "DATA TRANSFER ERROR - DISCARDING PACKET " << this->packetSize << "/" << buffLen << endl;
			//ii--;
			//goto end;
			shouldSkip = true;
		}
		if (this->iteration != iteration)
		{
			cout << "WRONG ITERATION DATA " << this->iteration << "/" << iteration << endl;
			shouldSkip = true;

		}
		else
		{
			if (shouldSkip)
				return shouldSkip;
			cout << "DATA TRANSFER VECTOR LEN: " << this->packetSize << endl;

			int lastIndex = 0;
		//	cout << "DPT" << endl;
			for (int kk = 0; kk < this->packetSize; ++kk)
			{
				
				cellDataStruct* dt = new cellDataStruct(ttb->getX(), ttb->getY(), ttb->lifeTime, ttb->state, ttb->mutationCounter);
				//dt->setState(ttb->getState());
				this->data.push_back(dt);
				//cout << dt->getX() << " TTT " << dt->getY() << endl;
			//	system("Pause");
				ttb++;
			}
			/*
			for (int xx = 0; xx < this->data.size(); xx++)
			{

				if (this->data[xx]->getX() == 0 && this->data[xx]->getY() == 0)
				{
					std::cout << "DATA MULT " << this->data[xx]->getX() << " " << this->data[xx]->getY() << " " << xx << endl;
					system("Pause");
				}
			}*/
		}
		return shouldSkip;
	}

};



#define STATUS_FREE 0
#define STATUS_WORKING 1
#define STATUS_ERROR 2

typedef struct workerData
{
	uint32_t serverId;
	SOCKADDR_IN serverAddr;
	SOCKET sock;
	uint8_t minPriority;
	uint8_t threads;
	uint16_t ram;
	uint8_t status;
	uint32_t lastPacketSize;
	void makeWorkerDataFromRQ(joinWorkerPoolRequest* rq)
	{
		minPriority = rq->minPriority;
		threads = rq->threads;
		ram = rq->ram;
	};
};
typedef struct workerPoolData
{
	/*
		ADD SIMULATION INIT DATA -> n, t...
	*/
	//-------------------------------------------------
	int cellNumber = 0;
	int iterations = 0;
	//-------------------------------------------------
	std::vector<workerData*>* wPool;
	int getWorkerPoolThreadNO()
	{
		int threadNO = 0;
		for (int ii = 0; ii < wPool->size(); ++ii)
		{
			threadNO += wPool->at(ii)->threads;
		}
		return threadNO;
	}
};
#pragma endregion;
typedef struct ClientInf
{
	int id;
	SOCKADDR_IN addr;
	SOCKET socket;
	char login[16];
};
class Server
{
private:
	SOCKET clientSocket[2];
public:
	std::vector<SOCKET> socketVector;
	std::vector<WSAEVENT> eventVector;
	std::vector<ClientInf*> clientInfoVect;


	std::vector<WSAEVENT> workerPoolEventVector;
	std::vector<workerData*> workerPoolVector;
	std::vector<SOCKET> workerPoolSocketVector;

	bool Initialize();
	void handleWorkerAccept(SOCKET workerSocket, SOCKADDR_IN clientAddr);
	void handleClientAccept(SOCKET workerSocket, SOCKADDR_IN clientAddr);
	void AcceptClient(int ii);

	bool StartServer();
	Server();
	~Server();

	uint32_t GetFirstAvaiableId();
	uint32_t GetFirstAvaiableServerId();
	//SEND IS SERVER OR CLIENT CONNECTING
	bool SISOCC(SOCKET sock);

};
std::vector<cellDataStruct*> splitData(std::vector<cellDataStruct*> dataSet, int iteration, int relativeSize);
//SEND REQUEST RESULT
bool SRR(SOCKET sock, bool result);
//SEND LOGIN RESULT
bool SLR(SOCKET sock, uint32_t netId);
//SEND HOST SERVER RESULT
void SHSR(SOCKET sock, Server* serv);

double calculateTime(LARGE_INTEGER recentTime, LARGE_INTEGER prevTime, LARGE_INTEGER cloak)
{
	return (double)((recentTime.QuadPart - prevTime.QuadPart) / (double)(cloak.QuadPart) * 1000);
}
#endif