// parts of this class are due to
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms737593(v=vs.85).aspx

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define TERMINATE "TERMINATE"

class Server {
private:
	PCSTR port;
	SOCKET ClientSocket;
	HWND* handle;
public:
	Server(char*, HWND*);
	~Server();
	int init();
	void start_comm();
};