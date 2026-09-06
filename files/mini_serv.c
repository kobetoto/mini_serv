#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

void error(){
  write(2, "Fatal error\n", 12);
  exit(1);
}

void broadcast(struct pollfd *fds, int nfds, int skip, char *msg, int len){
  for(int k = 1; k < nfds; k++)        // k = 1 : la case 0 est l'accueil, jamais
    if(k != skip)                      // tout le monde sauf un
      send(fds[k].fd, msg, len, 0);
}

int main(int ac, char **av){

  if(ac != 2){
    write(2, "Wrong number of arguments\n", 26);
    exit(1);
  } 

  //socket 
  int socket_serv = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_serv == -1)
    error();

  //bind
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(av[1]));
  addr.sin_addr.s_addr = htonl(2130706433);
  if(bind(socket_serv, (const struct sockaddr *)&addr, sizeof(addr)) == -1){
    close(socket_serv);
    error();
  }

  //listen
  if(listen(socket_serv, 42) == -1){
    close(socket_serv);
    error();
  }

  //poll()
  struct pollfd fds[100];
  int nfds = 0;
  fds[0].fd = socket_serv;
  fds[0].events = POLLIN;
  fds[0].revents = 0;
  nfds++;

  char  buf[100][1024];
  int   lens[100];
  int   ids[100];
  int   next_id = 0;

  for(;;){

    if(poll(fds, nfds, -1) == -1){
      close(socket_serv);
      error();
    }

    char  out[1100]; // the box 
    int   len = 0;

    for(int i = 0; i < nfds; i++){

      if(!(fds[i].revents & POLLIN))
        continue;

      //Client connection 
      if(fds[i].fd == socket_serv){

        int socket_cli = accept(socket_serv, NULL, NULL);
        if(socket_cli == -1)
          continue;

        fds[nfds].fd = socket_cli;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        lens[nfds] = 0;
        ids[nfds] = next_id++;
        ++nfds;

        len = sprintf(out, "server: client %d just arrived\n", ids[nfds - 1]);
        broadcast(fds, nfds, nfds - 1, out, len); // tous sauf le nouveau
      }

      //client request
      else{
        ssize_t n = recv(fds[i].fd, &buf[i][lens[i]], 1023-lens[i], 0);

        //client out
        if(n <= 0){
          len = sprintf(out, "server: client %d just left\n", ids[i]);
          broadcast(fds, nfds, i, out, len);
          close(fds[i].fd);

          //swap
          fds[i] = fds[nfds - 1];
          lens[i] = lens[nfds - 1];
          ids[i] = ids[nfds - 1];
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
              len = sprintf(out, "client %d: %s\n", ids[i], &buf[i][start]);
              broadcast(fds, nfds, i, out, len); // tous sauf l'auteur
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
