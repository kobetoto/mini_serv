#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

void error(int debug){
  fprintf(stderr,"[%i] ", debug);
  write(2, "Fatal Error\n", 12);
  exit(1);
}

int main(int ac, char **av){
  
  if(ac != 2)
    error(1);
  
  //socket 
  int socket_serv = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_serv == -1)
    error(2);
  
  //bind
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(av[1]));
  addr.sin_addr.s_addr = INADDR_ANY;
  if(bind(socket_serv, (const struct sockaddr *)&addr, sizeof(addr)) == -1){
    close(socket_serv);
    error(3);
  }

  //listen
  if(listen(socket_serv, 42) == -1){
    close(socket_serv);
    error(4);
  }
    
  fprintf(stdout, "[SERVER] ~READY~\n");

  //poll()
  struct pollfd fds[100];
  int nfds = 0;
  fds[0].fd = socket_serv;
  fds[0].events = POLLIN;
  fds[0].revents = 0;
  nfds++;
  
  char  buf[100][1024];
  int   lens[100];
  for(;;){
    if(poll(fds, nfds, -1) == -1){
      close(socket_serv);
      error(5);
    }

    for(int i = 0; i < nfds; i++){

      if(!(fds[i].revents & POLLIN))
        continue;

      //Client connection 
      if(fds[i].fd == socket_serv){
        
        int socket_cli = accept(socket_serv, NULL, NULL);
        if(socket_cli == -1){
          perror("accept");
          continue;
        }
        fds[nfds].fd = socket_cli;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        lens[nfds] = 0;
        ++nfds;
        fprintf(stdout, "[SERVER] Client #%d is connect\n[SERVER] Total Clients: %d\n", socket_cli, nfds);
      }

      //client request
      else{
        ssize_t n = read(fds[i].fd, &buf[i][lens[i]], 1023-lens[i]);
        
        //client out
        if(n <= 0){
          fprintf(stdout,"[SERVER] Client #%d exit\n", fds[i].fd);
          close(fds[i].fd);

          //swap
          fds[i] = fds[nfds - 1];
          lens[i] = lens[nfds - 1];
          memcpy(buf[i], buf[nfds - 1], 1024);
          --nfds;
          --i;
        }
        //client speak
        else{
          lens[i] += n;

          int start = 0;

          for(int j = 0; j < lens[i]; j++){
            if(buf[i][j] == '\n'){
              buf[i][j] = '\0';
              fprintf(stdout, "[CLIENT #%d] %s\n", fds[i].fd, &buf[i][start]);
              start = j+1;
            }
          }

          if(start > 0){
            memmove(buf[i], &buf[i][start], lens[i] - start);
            lens[i] -= start;
          }
        }
      }
    }
  }
  return (0);
}
