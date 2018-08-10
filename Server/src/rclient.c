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

int usrIntVal = 0;

void usrInt(int sig)
{
  usrIntVal = 1;
}

int main(int argc, char* argv[])
{
  signal(SIGINT, usrInt);
  int sock = 0;
  int port = 0;
  struct sockaddr_in servaddr;
  sock = socket(AF_INET, SOCK_STREAM, 6);
  int status = 0;
  char buffer[32] = "";
  if (sock == -1)
  {
    printf("could not establish connection\n");
    exit(1);
  }

  port = 5001;
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = htons(port);
  status = connect(sock, (struct sockaddr*) &servaddr, sizeof(servaddr));
  if (status == 0)
  {
    printf("connection is established successfully\n");
  }
  else
  {
    printf("could not run the app\n");
    exit(1);
  }

  status = read(sock, buffer, sizeof(buffer));
  if (status > 0)
  {
    printf("%d: %s\n", status, buffer);
    printf("Enter value X###Y### to move servo or 'exit' to quit\n");
  }

  char str[256];

  int st;
  do
  {
    fd_set f;
    FD_ZERO(&f);
    FD_SET(fileno(stdin), &f);
    st = select(fileno(stdin)+1, &f, NULL, NULL, NULL);
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
      write(sock, substr, sizeof(substr));
      if (strcmp(substr, "exit") == 0)
      {
        usrIntVal = 1;
      }
    }
  } while (!usrIntVal && st >= 0);

  close(sock);
  exit(EXIT_SUCCESS);
  return 0;
}
