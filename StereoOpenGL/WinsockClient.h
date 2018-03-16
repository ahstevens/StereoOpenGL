#pragma once

#include <string>

#include <WinSock2.h>

class WinsockClient
{
public:
	WinsockClient();
	~WinsockClient();

	bool connect(std::string server, int port);
	bool disconnect();
	bool send(std::string msg);

private:
	SOCKET m_Socket;
};

