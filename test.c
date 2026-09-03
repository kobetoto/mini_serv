#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>


void broadcast(struct pollfd *fds,int nfds, int skip, char *msg, int len){
  for(int i = 1; i < nfds; i++)
    if(i != skip)
      send(fds[i].fd, msg, len, 0);
}

void error(){
  write(2,"Fatal error\n", 12);
  exit(1);
  return;
}

int main(int ac, char **av){
  if(ac != 2)
    error();

  //part1
  int socket_serv = socket(AF_INET, SOCK_STREAM,0);
  if(socket_serv == -1)
    error();

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(av[1]));
  addr.sin_addr.s_addr = htonl(2130706433);//? 21..... ou le trouver dans les man?

  if (bind(socket_serv, (const struct sockaddr *)&addr, sizeof(addr)) == -1){
    close(socket_serv);
    error();
  }

  if(listen(socket_serv, 42) == -1){
    close(socket_serv);
    error();
  }


  //part2
  struct pollfd fds[100];
  int nfds = 0;
  fds[0].fd = socket_serv;
  fds[0].events = POLLIN;
  fds[0].revents = 0;
  nfds++;

  char  buf[100][10000];
  int   lens[100];
  int   ids[100];
  int   next_id = 0;

  char box[11000];
  int len = 0;


  for(;;){

    if(poll(fds, nfds, -1) == -1){
      close(socket_serv);
      error();
    }

    for(int i = 0; i < nfds; i++){

      if(!(fds[i].revents & POLLIN))
        continue;



      if(fds[i].fd == socket_serv){
        int socket_cli = accept(socket_serv, NULL, NULL);
        if(socket_cli == -1)
          continue;
        fds[nfds].fd = socket_cli;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        lens[nfds] = 0;
        ids[nfds] = next_id;
        next_id++;
        nfds++;

        len = sprintf(box, "server: client %d just arrived\n", ids[nfds -1]);
        broadcast(fds, nfds, nfds-1, box, len);
      }



      else{

        ssize_t n = recv(fds[i].fd, &buf[i][lens[i]], 10000-lens[i],0);

        if(n <= 0){
          len = sprintf(box, "server: client %d just left\n", ids[i]);
          broadcast(fds, nfds, i, box, len);
          close(fds[i].fd);

          fds[i] = fds[nfds-1];
          lens[i] = lens[nfds-1];
          ids[i] = ids[nfds-1];
          memcpy(buf[i], buf[nfds-1], 10000);
          --nfds;
          --i;
        }

        else{
          lens[i] += n;

          int start = 0;
          for(int j = 0; j < lens[i]; j++){
            if(buf[i][j] == '\n'){
              buf[i][j] = '\0';
              len = sprintf(box, "client %d: %s\n", ids[i], &buf[i][start]);
              broadcast(fds, nfds, i, box, len);
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
