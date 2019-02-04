/*
#pragma once

#include "../PacketManagement\PacketManagement.h"
#include "../DB\databasescripts.h"

#define NW_CLI_LOGIN_RD 0 
#define NW_SRV_LOGIN_SDC 1

using namespace NW;

EXPO DWORD WINAPI lnWorkerThread(LPVOID param)
{
	WriteLog("WORKER THREAD \n");
	int threadID = (int)param;

	OVERLAPPED* overlap = NULL;
	ClientInfo* clit = NULL;
	void* check = NULL;
	DWORD dwBytesTransfered = 0;
	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
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

		int nBytesRecv = 0;
		switch (clit->GetOPT())
		{
		case NW_CLI_LOGIN_RD:
			WriteLog("LOGIN RD\n");
			if ((WSARecv(clit->GetSocket(), p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL) == SOCKET_ERROR) && (WSA_IO_PENDING != WSAGetLastError()))
			{
				WriteLog("RECV ERROR: %d (ID: %d)\n", WSAGetLastError(), clit->GetID());
				
			}
			else
			{
				WriteLog("DATA RECEIVED FROM CLIENT %d\n", clit->GetID());
				WriteLog("LOGIN AUTH\n");
				WriteLog("DATA: %d\n", p_wbuf->len);
				LoginData* ldat = new LoginData;
				Deserialize(p_wbuf->buf, ldat);
				WriteLog("LOGIN: %s\n", ldat->login);
				WriteLog("LOGLEN: %d\n", ldat->logLen);
				WriteLog("PASS: %s\n", ldat->password);
				WriteLog("PASSLEN: %d\n", ldat->passLen);
				
				if (login(ldat->login, ldat->password))
				{
					WriteLog("LOGIN SUCCESSFUL\n");
					p_wbuf->buf = "SUCCESS";
					p_wbuf->len = strlen("SUCCESS");
				}
				else
				{
					WriteLog("LOG AUTH FAILED!!!\n");
					p_wbuf->buf = "FAILURE";
					p_wbuf->len = strlen("FAILURE");
				}
				if ((WSASend(clit->GetSocket(), p_wbuf, 1, &dwBytes, dwFlags, p_ol, NULL) == SOCKET_ERROR) && (WSA_IO_PENDING != WSAGetLastError()))
				{
					WriteLog("SEND DATA TO CLIENT ERROR: %d (ID: %d)\n ", WSAGetLastError(), clit->GetID());
				}
			
				
			}
			closesocket(clit->GetSocket());
			RemoveFromClientList(clit);
			
			break;
		default:
			break;
		}
	}
	return 0;
}
*/