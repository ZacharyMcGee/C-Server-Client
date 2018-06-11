#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>

#define PORT 3333
#define REMPS "remps"
#define SECRET "CS30618spr"
#define READY "ready"
#define USER "user"
#define CPU "cpu"
#define MEM "mem"

const char * remps = "<" REMPS ">";
const char * secret = "<" SECRET ">";
const char * ready = "<" READY ">";
const char * user = "<" USER ":";
const char * mem = "<" MEM ">";
const char * cpu = "<" CPU ">";

void handle_client(int connect_fd);

void handle_client(int connect_fd){
  int message_read;
  char buffer[1024];
  char *hello = "Hello from server";
  char * username = NULL;

  // Send <remps> protocol
  send(connect_fd, remps , strlen(remps) , MSG_NOSIGNAL);

  message_read = read(connect_fd , buffer, 1024);
  buffer[message_read] = '\0';

  // Check secret
  if(strcmp(buffer, secret) != 0){
    perror("Secret incorrect");
    exit(EXIT_FAILURE);
  }

  // Send <ready>
  send(connect_fd, ready, strlen(ready), MSG_NOSIGNAL);

  int usernameBuffer[255];
  int startUser = 0;
  int pos = 0;


  message_read = read(connect_fd, buffer, 1024);
  buffer[message_read] = '\0';

  // Check directive
  if(strncmp(buffer, user, 6) == 0){
    struct passwd *p;
    char username[300]; // On the exam you mentioned usernames are usually limited to 255
    int i = 6; // <user: makes the index 6

    while(i < strlen(buffer)){
      if(buffer[i] == '>'){
        username[pos] = '\0';
      }
      else{
        username[pos++] = buffer[i];
      }
      i++;
    }

    // Check that the username exists on the server
    if((p = getpwnam(username)) == NULL){
      exit(EXIT_FAILURE);
    }
    else
    {
      char command[300];
      char *commandoptions = " -o pid,ppid,%cpu,%mem,args";

      // Create command
      strcpy(command, "ps -u ");
      strcat(command, username);
      strcat(command, commandoptions);

      // Direct output to handle_client
      dup2(connect_fd, STDOUT_FILENO);
      dup2(connect_fd, STDERR_FILENO);

      // Call command
      system(command);
    }
  }
  // Option was <cpu>
  else if(strcmp(buffer, cpu) == 0){
    char command[] = "ps -NT -o pid,ppid,%cpu,%mem,args --sort -%cpu | head";
    dup2(connect_fd, STDOUT_FILENO);
    system(command);
  }
  // Option was <mem>
  else if(strcmp(buffer, mem) == 0){
    char command[] = "ps -NT -o pid,ppid,%cpu,%mem,args --sort -%mem | head";
    dup2(connect_fd, STDOUT_FILENO);
    system(command);
  }
  else
  {
    perror("Directive error");
    exit(EXIT_FAILURE);
  }

  close(connect_fd);
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int connect_fd;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("Sock option failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons( PORT );

    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // Inifinitly accept connections
    while(1){
      connect_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
      handle_client(connect_fd);
    }
    return 0;
}
