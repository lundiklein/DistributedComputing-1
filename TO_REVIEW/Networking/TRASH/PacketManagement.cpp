#include "PacketManagement\PacketManagement.h"

void Serialize(LoginData* rdata, char *data)
{

}
void Deserialize(char *data, LoginData* rdata)
{
	std::cout << "DESERIALIZATION" << std::endl;
	int* length = (int*)data;
	rdata->logLen = *length;
	length++;
	rdata->passLen = *length;
	length++;
	char* recData = (char*)length;
	int ii = 0;
	for (int ii = 0; ii < rdata->logLen; ii++)
	{

		rdata->login[ii] = *recData;
		recData++;
		std::cout << ii << " ";

	}
	for (int ii = 0; ii < rdata->passLen; ii++)
	{

		rdata->password[ii] = *recData;
		recData++;
		std::cout << ii << " ";

	}
}


void PrintMessage(LoginData* rdata)
{

	std::cout << "LLEN: " << rdata->logLen << std::endl;
	std::cout << "PLEN: " << rdata->passLen << std::endl;
	std::cout << "LOGIN: " << rdata->login << std::endl;
	std::cout << "PASSWORD: " << rdata->password << std::endl;
}
void Serialize(ClientList* list, char *data)
{
	int* id = (int*)data;
	*id = list->id;
	id++;
	std::string* clientIP = (std::string*)id;
	*clientIP = list->clientIP;
	clientIP++;
	Scene* scene = (Scene*)clientIP;
	*scene = list->scene;
	scene++;
}
void Deserialize(char *data, ClientList* list)
{
	int* id = (int*)data;
	list->id = *id;
	id++;
	std::string* clientIP = (std::string*)id;
	list->clientIP = *clientIP;
	clientIP++;
	Scene* scene = (Scene*)clientIP;
	list->scene = *scene;
	scene++;
}
void PrintMessage(ClientList* list)
{
	std::cout << "ID: " << list->id << " IP: " << list->clientIP << " SCENE: " << list->scene << std::endl;
}