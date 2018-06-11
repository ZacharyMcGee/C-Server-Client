#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>


#define PORT 3333
#define BUFFER_SIZE 4096;
#define REMPS "remps"
#define SECRET "CS30618spr"
#define READY "ready"
#define USERDIR "user"

const char * remps = "<" REMPS ">";
const char * secret = "<" SECRET ">";
const char * ready = "<" READY ">";
const char * user = "<" USERDIR ":";

typedef enum { USER, CPU, MEM } directive; // DIRECTIVE OPTIONS


int main(int argc, char const *argv[])
{
    directive frmt = USER; // Default is user

    // Check that we at least have the IP option
    if(argc > 3 || argc < 2){
      perror("USAGE: SERVER_IP_ADDRESS [user | cpu | mem]\n");
      exit(EXIT_FAILURE);
    }

    // We have a directive option
    if(argc == 3){
      if(strcmp(argv[2], "user") == 0){
        frmt = USER;
      }
      else if(strcmp(argv[2], "cpu") == 0){
        frmt = CPU;
      }
      else if(strcmp(argv[2], "mem") == 0){
        frmt = MEM;
      }
      else
      {
        perror("USEAGE: Directive must be one of: [user | cpu | mem]\n");
        exit(EXIT_FAILURE);
      }
    }

    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;

    int message_read;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_aton(argv[1], &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    message_read = read(sock, buffer, 1024);
    buffer[message_read] = '\0';

    // Send <remps> to server
    if(strcmp(buffer, remps) == 0){
      send(sock, secret, strlen(secret) , MSG_NOSIGNAL);
    }
    else
    {
      close(sock);
      perror("<remps> failed");
      exit(EXIT_FAILURE);
    }

    message_read = read(sock, buffer, 1024);
    buffer[message_read] = '\0';

    // Server sent <ready>
    if(strcmp(buffer, ready) == 0){
      struct passwd *pws;
      char useroption[300];

      // Check option
      switch(frmt){
        case USER:
          pws = getpwuid(getuid());
          char *username = (pws->pw_name); // Get the username

          // Create username str
          strcpy(useroption, "<user:");
          strcat(useroption, username);
          strcat(useroption, ">");

          send(sock, useroption, strlen(useroption), MSG_NOSIGNAL); // Send user option
          break;
        case CPU:
          send(sock, "<cpu>", strlen("<cpu>"), MSG_NOSIGNAL); // Send cpu option
          break;
        case MEM:
          send(sock, "<mem>", strlen("<mem>"), MSG_NOSIGNAL); // Send mem option
          break;
      }

      // Read until no bytes are left
      while((message_read = read(sock, buffer, 1024)) > 0){
        if(write(STDOUT_FILENO, buffer, message_read) > 0){
          if(message_read < 1024){
            break;
          }
        }
      }
    }
    else
    {
      close(sock);
      exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
