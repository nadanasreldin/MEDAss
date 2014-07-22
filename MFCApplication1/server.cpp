// parts of this class are due to
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms737593(v=vs.85).aspx

#include "stdafx.h"
#include "server.h"

Server::Server(char* portnum, HWND* handle) {
	this->port = portnum;
	this->handle = handle;
}

Server::~Server() {
	int iResult;
	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();
}

int Server::init() {

	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	this->ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, this->port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket(ListenSocket);

	return 0;
}

void Server::start_comm() {
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int iResult;
	// Receive until the peer shuts down the connection
	do {
		ZeroMemory(recvbuf, recvbuflen);
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("%s", recvbuf);

			/*wchar_t tempbuf[DEFAULT_BUFLEN];
			swprintf(tempbuf, strlen(recvbuf) + 1, L"%p", recvbuf);
			LPTSTR msg = tempbuf;*/

			/*int len = ::MultiByteToWideChar(CP_ACP, 0, recvbuf, lstrlenA(recvbuf), NULL, 0);
			wchar_t *tempbuf = new wchar_t[len];
			::MultiByteToWideChar(CP_ACP, 0, recvbuf, lstrlenA(recvbuf), tempbuf, 0);
			LPTSTR msg = tempbuf;*/

			// widen string to pass in message
			size_t size = strlen(recvbuf) + 1;
			wchar_t *tempbuf = new wchar_t[size];
			size_t convertedChars = 0;
			mbstowcs_s(&convertedChars, tempbuf, size, recvbuf, _TRUNCATE);
			//LPTSTR msg = tempbuf;

			// PostMessage(*this->handle, WM_NEW_MSG, (WPARAM)msg, 0);
			PostMessage(*this->handle, WM_NEW_MSG, 0, (LPARAM)tempbuf);

			// TODO: free tempbuf??? where? not here
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else  {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return;
		}

	} while (iResult > 0);
}

