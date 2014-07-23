// parts of this class are due to
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms737591(v=vs.85).aspx

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define TERMINATE "TERMINATE\n"

class Client {
private:
	char* hostname;
	SOCKET connectSocket;
public:
	Client(char* hostname) : hostname(hostname) {}
	~Client();
	int init();
	void startComm();
};