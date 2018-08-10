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
int deadSockVal = 0;

void usrInt(int sig)
{
	usrIntVal = 1;
}

void deadSock(int sig)
{
	deadSockVal = 1;
}

int main()
{
	WSADATA wsaData;
	int iResult;
	
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}
	signal(SIGINT, usrInt);
	signal(WSAECONNRESET, deadSock);
	
	// Raspberry Pi GPIO
	/*
	wiringPiSetupGpio();
	pinMode (18, PWM_OUTPUT);
	pwmSetMode (PWM_MODE_MS);
	pwmSetClock (400);
	pwmSetRange (1000);
	*/
	
	int opt = 1;

	struct addrinfo *result = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET sock = INVALID_SOCKET;

	sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (sock == INVALID_SOCKET) {
		printf("Error at socket(): %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	if ((setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const char *)&opt, sizeof(opt))) < 0)
	{
		perror("setsockopt");
		closesocket(sock);
		exit(EXIT_FAILURE);
	}

	struct linger {
		int l_onoff;
		int l_linger;
	} myLinger;

	myLinger.l_onoff = 1;
	myLinger.l_linger = 1;

	if ((setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&myLinger, sizeof(myLinger))) < 0)
	{
		perror("setsockopt");
		closesocket(sock);
		exit(EXIT_FAILURE);
	}

	if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt))) < 0)
	{
		perror("setsockopt");
		closesocket(sock);
		exit(EXIT_FAILURE);
	}

	if (sock == -1)
	{
		fprintf(stderr, "failed\n");
	}
	else
	{
		printf("connection is established\n");
	}

	// Setup the TCP listening socket
	iResult = bind(sock, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %d\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return 1;
	}
	
	SOCKET childSocket;
	childSocket = INVALID_SOCKET;
		
	while (1)
	{

		// Accept a client socket
		childSocket = accept(sock, NULL, NULL);
		if (childSocket == INVALID_SOCKET || childSocket == -1) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(sock);
			WSACleanup();
			return 1;
		}

		char message[] = "Connected to Servos!\n";

		send(childSocket, message, sizeof(message), 0);

		char recvbuf[DEFAULT_BUFLEN] = "";
		int iResult, iSendResult;
		int recvbuflen = DEFAULT_BUFLEN;
		int exitStr = 0;

		// Receive until the peer shuts down the connection
		do {

			iResult = recv(childSocket, recvbuf, recvbuflen, 0);
			
			char substr[strlen(recvbuf) + 1];
			
			if (iResult > 0)
			{
				int i;
				for (i = 0; i < (strlen(recvbuf)); i++)
				{
					substr[i] = recvbuf[i];
				}

				char four[] = { recvbuf[0], recvbuf[1], recvbuf[2], recvbuf[3], 0 };

				printf("%d: %s\n", iResult, substr);

				if (strcmp(four, "exit") == 0)
				{
					exitStr = 1;
				}
			}

			if (iResult > 0 && !exitStr) {
				int xVal;
				int yVal;
				char xValChar[4] = { substr[1], substr[2], substr[3], 0 };
				char yValChar[4] = { substr[5], substr[6], substr[7], 0 };
				int i;
				for (i = 0; i < sizeof(substr); i++)
				{
					printf("i: %i, char: %d\n", i, substr[i]);
				}
				sscanf(xValChar, "%d", &xVal);
				sscanf(yValChar, "%d", &yVal);
				double newXVal;
				double newYVal;
				newXVal = ((((double)xVal) / 180) * 84) + 36;
				newYVal = ((((double)yVal) / 180) * 84) + 36;
				printf("xVal: %i\n", xVal);
				printf("yVal: %i\n", yVal);
				printf("newXVal: %f\n", newXVal);
				printf("newYVal: %f\n", newYVal);
				//pwmWrite(18,newXVal);
			}
			else if (iResult == 0 || exitStr)
			{
				printf("Connection closing...\n");
				char msg[] = "exit";
				iSendResult = send(childSocket, msg, sizeof(msg), 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(childSocket);
					WSACleanup();
					return 1;
				}
				iResult = 0;
			}
			else {
				printf("recv failed: %d\n", WSAGetLastError());
				closesocket(childSocket);
				WSACleanup();
				return 1;
			}

		} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(childSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(childSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(childSocket);
	WSACleanup();

	return 0;
}
}