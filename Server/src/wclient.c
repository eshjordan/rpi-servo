#include <stdio.h>
#include <sys/types.h>
#include <Winsock2.h>
#include <winsock.h>
#include <Ws2tcpip.h>
#include <stdlib.h>
#include <unistd.h>
#include <io.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
//#include <wiringPi.h>
//#include <softPwm.h>

#define DEFAULT_PORT "5001"
#define DEFAULT_BUFLEN 512

int usrIntVal = 0;

void usrInt(int sig)
{
	usrIntVal = 1;
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Connect to server.
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	int recvbuflen = 0;

	char recvbuf[DEFAULT_BUFLEN] = "";

	int status = read(ConnectSocket, recvbuf, recvbuflen);
	printf("%d: %s\n", status, recvbuf);
	printf("Enter value X###Y### to move servo or 'exit' to quit\n");

	char str[DEFAULT_BUFLEN] = "";

	int st;
	do
	{
		fd_set f;
		FD_ZERO(&f);
		FD_SET(fileno(stdin), &f);
		st = select(fileno(stdin) + 1, &f, NULL, NULL, NULL);
		if (st == 1)
		{
			fgets(str, sizeof(str), stdin);
			char substr[strlen(str)];
			int i;
			for (i = 0; i < (strlen(str) - 1); i++)
			{
				substr[i] = str[i];
			}
			substr[strlen(str) - 1] = 0;
			write(ConnectSocket, substr, DEFAULT_BUFLEN);
			
			if (strcmp(substr, "exit") == 0)
			{
				usrIntVal = 1;
			}
		}
	} while (!usrIntVal || st >= 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;

}