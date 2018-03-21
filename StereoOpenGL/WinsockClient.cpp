#include "WinsockClient.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 5005


WinsockClient::WinsockClient()
	: m_Socket(INVALID_SOCKET)
{
	
}


WinsockClient::~WinsockClient()
{
}

bool WinsockClient::connect(std::string server, int port = DEFAULT_PORT)
{
	if (m_Socket != INVALID_SOCKET)
		disconnect();

	WSADATA wsaData;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;

	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return false;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(server.c_str(), std::to_string(port).c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return false;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		m_Socket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (m_Socket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return false;
		}

		// Connect to server.
		iResult = ::connect(m_Socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (m_Socket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return false;
	}

	printf("Connected to %s\n", server);

	return true;
}

bool WinsockClient::disconnect()
{
	// shutdown the connection since no more data will be sent
	int iResult = shutdown(m_Socket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("Disconnect failed with error: %d\n", WSAGetLastError());
		closesocket(m_Socket);
		WSACleanup();
		return false;
	}

	return true;
}

bool WinsockClient::send(std::string msg)
{
	// Send an initial buffer
	int iResult = ::send(m_Socket, msg.c_str(), (int)msg.length(), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(m_Socket);
		WSACleanup();
		return false;
	}

	printf(">>> Message Sent (%ld Bytes):\n\t%s\n\n", iResult, msg);

	// Receive echo response
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	iResult = recv(m_Socket, recvbuf, recvbuflen, 0);
	recvbuf[iResult] = '\0';
	if (iResult > 0)
		printf("<<< Message Received (%ld Bytes):\n\t%s\n\n", iResult, recvbuf);
	else if (iResult == 0)
		printf("Connection closed\n");
	else
		printf("recv failed with error: %d\n", WSAGetLastError());

	return true;
}
