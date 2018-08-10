#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/io.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <wiringPi.h>
#include <softPwm.h>

const char message[] = "Connected to Servos!\n";

int sock;

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

int roundNo(double num)
{
    return num < 0 ? num - 0.5 : num + 0.5;
}

int main()
{
  signal(SIGINT, usrInt);
  signal(SIGPIPE, deadSock);
  /**/
  wiringPiSetupGpio();
  pinMode (18, PWM_OUTPUT);
  pinMode (17, PWM_OUTPUT);
  pwmSetMode (PWM_MODE_MS);
  pwmSetClock (400);
  pwmSetRange (1000);
  /**/
  int opt = 1;
  int sock = 0;
  int port = 5001;
  char buffer[32] = "";

  if ((sock = socket(AF_INET, SOCK_STREAM, 6))==0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  if ((setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt))) < 0)
  {
    perror("setsockopt");
    close(sock);
    exit(EXIT_FAILURE);
  }

  struct linger {
    int l_onoff;
    int l_linger;
  } myLinger;

  myLinger.l_onoff = 1;
  myLinger.l_linger = 1;

  if ((setsockopt(sock, SOL_SOCKET, SO_LINGER, &myLinger, sizeof(myLinger))) < 0)
  {
    perror("setsockopt");
    close(sock);
    exit(EXIT_FAILURE);
  }

  if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) < 0)
  {
    perror("setsockopt");
    close(sock);
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

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(port);

  int status = bind(sock, (struct sockaddr*) &server, sizeof(server));
  if (status == 0)
  {
    printf("connection completed\n");
  }
  else
  {
    printf("problem is encountered\n");
  }
  status = listen(sock, 5);
  if (status == 0)
  {
    printf("app is ready to work\n");
  }
  else
  {
    printf("connection is failed\n");
    return 0;
  }

  while (usrIntVal != 1)
  {
    struct sockaddr_in client = { 0 };
    socklen_t len = sizeof(client);
    int childSocket = accept(sock, (struct sockaddr*) &client, &len);
    if (childSocket == -1)
    {
      printf("cannot accept connection\n");
      close(childSocket);
    }
    else
    {
      printf("IP address is: %s\n", inet_ntoa(client.sin_addr));
      printf("port is: %d\n", (int) ntohs(client.sin_port));

      write(childSocket, message, sizeof(message));
      while (!usrIntVal && !deadSockVal)
      {
        int readVal = read(childSocket, buffer, sizeof(buffer));
        char substr[strlen(buffer) + 1];
        int i;
        for (i = 0; i < (strlen(buffer)); i++)
        {
          substr[i] = buffer[i];
        }
        substr[strlen(buffer)] = 0;

        if (readVal > 0)
        {
          char subsub[] = {buffer[0], buffer[1], buffer[2], buffer[3], 0};
          printf("%d: %s\n", readVal, substr);
          if (strcmp(subsub, "exit") == 0)
          {
            deadSockVal = 1;
          }
          else
          {
            int xVal;
            int yVal;
            char xValChar[4] = {substr[1], substr[2], substr[3], 0};
            char yValChar[4] = {substr[5], substr[6], substr[7], 0};
            int i;
            for (i = 0; i < sizeof(substr); i++)
            {
              printf("i: %i, char: %d\n", i, substr[i]);
            }
            sscanf(xValChar, "%d", &xVal);
            sscanf(yValChar, "%d", &yVal);
            double newXVal;
            double newYVal;
            newXVal = ((((double)xVal)/180)*84) + 36;
            newYVal = ((((double)yVal)/180)*84) + 36;
            printf("xVal: %i\n", xVal);
            printf("yVal: %i\n", yVal);
			xVal = roundNo(newXVal);
			yVal = roundNo(newYVal);
            printf("newXVal: %i\n", xVal);
            printf("newYVal: %i\n", yVal);

            pwmWrite(18,xVal);
			pwmWrite(17, yVal);
          }
        }
      }
      close(childSocket);
    }
    deadSockVal = 0;
  }

  close(sock);

  char sockFile[] = "";
  remove(sockFile);

  return 0;
}
