#pragma once
#ifdef _BSD_SERVER_H


#define NW_CLI_LOGIN_RD 0 
#define NW_SRV_LOGIN_SDC 1

using namespace NW;

EXPO DWORD WINAPI WorkerThread(LPVOID param)
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
		WSABUF *bsd_wbuf;
		ZeroMemory(&bsd_wbuf, sizeof(WSABUF));
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

				BSDCON(BSD_PI, BSD_TROP, clit, bsd_wbuf, p_wbuf, p_ol, dwBytes, dwFlags);
			}
			break;
		case NW_SRV_LOGIN_SDC:
			WriteLog("LOGIN SDC\n");
			if ((WSASend(clit->GetSocket(), bsd_wbuf, 1, &dwBytes, dwFlags, p_ol, NULL) == SOCKET_ERROR) && (WSA_IO_PENDING != WSAGetLastError()))
			{
				WriteLog("SEND DATA TO CLIENT ERROR: %d (ID: %d)\n ", WSAGetLastError(), clit->GetID());
			}
			if (strcmp(bsd_wbuf->buf, "CONNECTED") == 0)
			{
				closesocket(clit->GetSocket());
				WriteLog("CLIENT AUTH SUCCESS ID: %d\n", clit->GetID());
				RemoveFromClientList(clit);
				break;
			}
			else
			{
				WriteLog("CLIENT AUTH FAILED ID: %d\n", clit->GetID());
				clit->SetOPT(NW_CLI_LOGIN_RD);
			}

		default:
			break;
		}
	}
	return 0;
}

#endif