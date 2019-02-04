/*
#pragma once

#include "../PacketManagement\PacketManagement.h"
#include "../DB\databasescripts.h"
#include "../../Player/Character.h"
#define NW_CS_RD 0  //GET ACTION INFO FROM CLIENT
#define NW_CS_GHL 1	//GET CHARACTER LIST
#define NW_CS_EXT 2 //CLIENT EXIT

using namespace NW;
EXPO DWORD WINAPI csWorkerThread(LPVOID param)
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
		
		CharData* cdat = new CharData;
		int nBytesRecv = 0;
		bool clientOperation = true;
		while (clientOperation)
		{
		
			switch (clit->GetOPT())
			{
			case NW_CS_RD:
				WriteLog("CHAR RD\n");
				if ((WSARecv(clit->GetSocket(), p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL) == SOCKET_ERROR) && (WSA_IO_PENDING != WSAGetLastError()))
				{
					WriteLog("RECV ERROR: %d (ID: %d)\n", WSAGetLastError(), clit->GetID());
					clit->SetOPT(NW_CS_EXT);
					break;
				}
				else
				{
					WriteLog("CHAR_RD RCV ID: %d\n", clit->GetID());

					if (csGetCharOpt(p_wbuf->buf) == 0) // GET CHAR INFO
					{
						clit->SetOPT(NW_CS_GHL);
						break;
					}
					else if (csGetCharOpt(p_wbuf->buf) == 1) // CREATE CHARACTER
					{
						WriteLog("CHAR CRT\n");
						//deserialize info
						csDeserialize(p_wbuf->buf, cdat);
						if (createCharacter(cdat->login, cdat->player[0].name))
						{
							WriteLog("CHARACTER CREATED ACC: %s ; NAME: %s\n", cdat->login, cdat->player[0].name);
							p_wbuf->buf = "SUCCESS";
							p_wbuf->len = strlen("SUCCESS");
						}
						else
						{
							WriteLog("CRT ERROR\n");
							WriteLog("LOGIN: %s\n", cdat->login);
							WriteLog("CHAR: %s\n", cdat->player->name);
							p_wbuf->buf = "FAILURE";
							p_wbuf->len = strlen("FAILURE");
						}
						//RESPONSE TO CLIENT
						//TODO NEW CASE SEND!!
						if ((WSASend(clit->GetSocket(), p_wbuf, 1, &dwBytes, dwFlags, p_ol, NULL) == SOCKET_ERROR) && (WSA_IO_PENDING != WSAGetLastError()))
						{
							WriteLog("SEND DATA TO CLIENT ERROR: %d (ID: %d)\n ", WSAGetLastError(), clit->GetID());
						}
						else
						{
							WriteLog("RESPONSE CRT SEND\n");
						}
						
						clit->SetOPT(NW_CS_GHL);
						break;
					}
					
				}
				break;

			case NW_CS_GHL:
				WriteLog("CHAR GHL\n");
				//GET CHARACTER LIST FROM DB
				//SERIALIZE IT
				//SEND IT
				csDeserialize(p_wbuf->buf, cdat);
				characterLoad(cdat->login, cdat->player);
				std::cout << "LOGIN: " << cdat->login << std::endl;
				std::cout << "CHAR1: " << cdat->player[0].name << " LVL: " << cdat->player[0].lvl << std::endl;
				std::cout << "CHAR2: " << cdat->player[1].name << " LVL: " << cdat->player[1].lvl << std::endl;
				std::cout << "CHAR3: " << cdat->player[2].name << " LVL: " << cdat->player[2].lvl << std::endl;
				std::cout << "CHAR4: " << cdat->player[3].name << " LVL: " << cdat->player[3].lvl << std::endl;
				csSerialize(cdat, p_wbuf->buf);
				if ((WSASend(clit->GetSocket(), p_wbuf, 1, &dwBytes, dwFlags, p_ol, NULL) == SOCKET_ERROR) && (WSA_IO_PENDING != WSAGetLastError()))
				{
					WriteLog("SEND DATA TO CLIENT ERROR: %d (ID: %d)\n ", WSAGetLastError(), clit->GetID());
				}
				else
				{
					WriteLog("DATA SENT\n");
				}
				RemoveFromClientList(clit);
				clit->SetOPT(NW_CS_EXT);
				break;
				//GET EXIT LOGIN SERV RESPONSE
			case NW_CS_EXT:
				WriteLog("EXIT\n");
				closesocket(clit->GetSocket());
				clientOperation = false;
				
				break;
				//TERMINATE CLIENT
			default:
				
				break;
			}
			
		}
		
	}
	return 0;
}
*/