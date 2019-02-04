#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_CLIENT_NUMBER 1000
#define DEFAULT_BUFLEN 1024
#define MAX_LOGIN_SIZE 64
#define MAX_PASS_SIZE 64
#define LOGIN_SERVER_PORT 6635
#define CHAR_SERVER_CLIENT_PORT 6654
#define CHAR_SERVER_PORT 6633
#define LOGIN_SERVER_IP "127.0.0.1"
#define CHAR_SERVER_IP "127.0.0.1"
