#include "stdafx.h"
#include "NetworkCustom.h"

int main()
{
	SOCKET sock = INVALID_SOCKET;
	if (JoinWorkerPool(sock))
	{
		std::cout << "JWP S" << std::endl;
	}
	system("Pause");
	return 0;
}