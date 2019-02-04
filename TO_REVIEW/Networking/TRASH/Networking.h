#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#include "PacketManagement.h"

#define NO_OF_THREADS_PER_PROCESOR 2
#define WAIT_INTERV 100
#define OPT_READ 0
#define OPT_WRITE 1

extern int maxThreads = 0; 
//------------------HANDLE--------SECTION-------------------
HANDLE h_IOCP = NULL;  __declspec(dllexport)
HANDLE* h_WORKER_THREADS = NULL;  __declspec(dllexport)
HANDLE h_ACCEPT_EVENT = NULL;  __declspec(dllexport)
HANDLE h_SHUTDOWN = NULL;  __declspec(dllexport)
//----------------------------------------------------------
class __declspec(dllexport) Networking
	{


	private:
		PacketManagement packetManager;
	

	public:

		//void AcceptConnections(SOCKET &connectionSocket, SOCKET &receiveSocket);
		bool RecLoginData(SOCKET receiveSocket, char *data, int dataLen, int flags, LoginData* rdata);
		void Send(SOCKET clientSocket, char* sendInfo);
		void ConnectTo(SOCKET &clientSocket, char* ipAddress, int port);
		//	void SetupReceiveSocket(SOCKET &clientSocket, int port);
		void SetupUDPSocket(SOCKET &clientSocket, int port);
		void SetupMulticastSendSocket(SOCKET &clientSocket, char* multicastIP, int multicastPort, SOCKADDR_IN &multicastAddr);
		void SetupReceiveMCSocket(SOCKET &receiveSocket, char* multicastIP, int multicastPort, SOCKADDR_IN &mcAddr, ip_mreq &mcJoin);
		void ConnectedClients(SOCKET clientSocket, SOCKADDR_IN peerAddr, std::list<ClientList>& cl, int& id, Scene scene);
		void PrintConnectedClientsList(std::list<ClientList> clients);

		


	};
/*
TODO
	-INITIALIZATION
	-IOCP INIT
	-ACCEPT CONNECTION
	-THREAD HANDLERS
*/
class ClientInfo
{
private:
	OVERLAPPED *overlap;
	WSABUF *wsabuf;
	SOCKET clientSocket;
	int bytesSent;
	int bytesTotal;
	char buffer[DEFAULT_BUFLEN];
	int opt;
public:
	int GetBytesSent()
	{
		return bytesSent;
	}
	void UpdBytesSent(int n)
	{
		bytesSent += n;
	}
	void SetBytesSent(int n)
	{
		bytesSent = n;
	}
	int GetBytesTotal()
	{
		return bytesTotal;
	}
	void SetBytesTotal(int n)
	{
		bytesTotal = n;
	}
	int GetOPT()
	{
		return opt;
	}
	void SetOPT(int n)
	{
		opt = n;
	}
	SOCKET GetSOCKET()
	{
		return clientSocket;
	}
	void SetSOCKET(SOCKET s)
	{
		clientSocket = s;
	}

	void GetBuffer(char* msg)
	{
		strcpy(msg, buffer);
	}
	void SetBuffer(char* msg)
	{
		strcpy(buffer, msg);
	}
	void ZeroBuffer()
	{
		ZeroMemory(buffer, DEFAULT_BUFLEN);
	}


	OVERLAPPED* GetOVERLAPPED()
	{
		return overlap;
	}
	WSABUF* GetWSABUF()
	{
		return wsabuf;
	}
	int GetWSABUFLen()
	{
		return wsabuf->len;
	}
	void SetWSABUFLen(int len)
	{
		wsabuf->len = len;
	}
	void ResetWSABUF()
	{
		ZeroBuffer();
		wsabuf->buf = buffer;
		wsabuf->len = DEFAULT_BUFLEN;
	}


};
bool Init();  __declspec(dllexport)
bool InitIOCP();  __declspec(dllexport) 
bool ConnectWithIOCP(ClientInfo* clinfo);  __declspec(dllexport)
bool SetupReceiveSocket(SOCKET &clientSocket, int port);  __declspec(dllexport)
void AcceptConnections(SOCKET clientSocket);  __declspec(dllexport)
bool AcceptEvent(SOCKET clientSocket, HANDLE handle);  __declspec(dllexport)

DWORD WINAPI AcceptThread(LPVOID param); __declspec(dllexport)
DWORD WINAPI WorkerThread(LPVOID param); __declspec(dllexport)
#endif