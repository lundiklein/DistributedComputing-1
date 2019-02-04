#include "Network.h"
//----LOGIN DATA
void Serialize(LoginData* rdata, char *data);
void Deserialize(char *data, LoginData* rdata);
//----CHAR DATA
void Serialize(ClientList* list, char *data);
void Deserialize(char* data, ClientList* list);
//----GAME DATA


//----MESSAGE PRINT
	void PrintMessage(LoginData* rdata);
	void PrintMessage(ClientList* list);
