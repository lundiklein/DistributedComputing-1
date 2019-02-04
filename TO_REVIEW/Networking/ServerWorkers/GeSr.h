#pragma once

#include "../PacketManagement\PacketManagement.h"
#include "../DB\databasescripts.h"

#define NW_GS_CTRL 0
using namespace NW;

DWORD WINAPI srecvThread(LPVOID param)
{
	WriteLog("RECV THREAD SETUP");
	ClientInfo* clit = (ClientInfo*)param;
	DWORD dwBytesTransfered = 0;
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	DWORD id;
	SOCKET recvSock = clit->GetSocket();
	WSABUF* p_wbuf = clit->GetWSABUF();
	OVERLAPPED* p_ol = clit->GetOVERLAPPED();
	SOCKADDR_IN addr = clit->GetAddr();
	while (true)
	{
		if ((WSARecvFrom(recvSock, p_wbuf, 1, &dwBytes, &dwFlags, (sockaddr*)&addr, (LPINT)sizeof(addr), p_ol, NULL) != SOCKET_ERROR) && (WSA_IO_PENDING != WSAGetLastError()))
		{
			//UPD PLAYER STATE
		}
	}
	return 0;
}
DWORD WINAPI ssendThread(LPVOID param)
{
	WriteLog("SEND THREAD SETUP");
	ClientInfo* clit = (ClientInfo*)param;
	DWORD dwBytesTransfered = 0;
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	DWORD id;
	SOCKET recvSock = clit->GetSocket();
	WSABUF* p_wbuf = clit->GetWSABUF();
	OVERLAPPED* p_ol = clit->GetOVERLAPPED();
	SOCKADDR_IN addr = clit->GetAddr();
	while (true)
	{
		if ((WSASendTo(recvSock, p_wbuf, 1, &dwBytes, dwFlags, (sockaddr*)&addr, sizeof(addr), p_ol, NULL) != SOCKET_ERROR) && (WSA_IO_PENDING != WSAGetLastError()))
		{
			//UPDATE CLIENT CHAR
		}
		Sleep(15);
	}
	return 0;
}
EXPO DWORD WINAPI gsWorkerThread(LPVOID param)
{
	WriteLog("WORKER THREAD \n");
	int threadID = (int)param;

	OVERLAPPED* overlap = NULL;
	ClientInfo* clit = NULL;
	void* check = NULL;
	DWORD dwBytesTransfered = 0;
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	DWORD id = 0;
	while (WAIT_OBJECT_0 != WaitForSingleObject(h_SHUTDOWN, 0))
	{
		BOOL que = GetQueuedCompletionStatus(h_IOCP, &dwBytesTransfered, (PULONG_PTR)&check, &overlap, INFINITE);
		if (check == NULL)
		{
			WriteLog("NO CLIENT TO WORK ON THREAD: %d\n", threadID);
			break;
		}
		else
		{
			WriteLog("GOT CLIENT ON THREAD %d\n", threadID);
		}

		clit = (ClientInfo*)check;
		if ((que == FALSE) || (0 == dwBytesTransfered))
		{
			WriteLog("CLIENT GONE ON THREAD %d\n", threadID);
			RemoveFromClientList(clit);
			continue;
		}
		WSABUF *p_wbuf = clit->GetWSABUF();
		OVERLAPPED *p_ol = clit->GetOVERLAPPED();
		HANDLE h_RECV = new HANDLE();
		HANDLE h_SEND = new HANDLE();
		int nBytesRecv = 0;
		switch (clit->GetOPT())
		{
		case NW_GS_CTRL:
	
		default:
			break;
		}
	}
	return 0; 
}