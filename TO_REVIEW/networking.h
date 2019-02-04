#pragma once
#ifndef _NETWORKING_H
#define _NETWORKING_H


bool setupTCPSocket(SOCKET* socket);
bool tcpConnect(SOCKET* clientSocket, std::string addr, int port);


#endif
