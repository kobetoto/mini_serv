#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>




int main(){

  //SOCKET
  int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
  if(serv_sock == -1){perror("Socket"); return (1);}

  //SOCKADDR
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  addr.sin_addr.s_addr = INADDR_ANY;

  //BIND
  if(bind(serv_sock, (const struct sockaddr*)&addr, sizeof(addr)) == -1){
    perror("Bind"); return(1);
  }

  //LISTEN
  if(listen(serv_sock, 10) == -1){
    perror("Listen"); return(1);}


  //POLL
  struct pollfd fds[100];
  int nfds = 0;

  fds[0].fd = serv_sock;
  fds[0].events = POLLIN;
  nfds++;

  //BUFF
  char buf[100][1024];
  int lens[100];

  //SERVER CORE
  for(;;){

    if(poll(fds, nfds, -1) == -1){perror("bind"); return(1);}

    for(int i = 0; i < nfds; i++){

      if(!(fds[i].revents & POLLIN))
        continue;

      //NEW CLIENT (SYN -> SERVER SOCKET)
      if(fds[i].fd == serv_sock){
        int cli_sock = accept(serv_sock, NULL, NULL);

        if(cli_sock == -1){perror("accept"); continue;}

        fds[nfds].fd = cli_sock;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        lens[nfds] = 0;
        nfds++;
        fprintf(stdout, "~client [%d] connected~\n~Nb of clients = %d~\n",cli_sock, nfds);
      }

      //CLIENT REQUEST
      else{

        ssize_t n = read(fds[i].fd, buf[i][lens[i]], 1023 - lens[i]);

        //CLIENT QUIT || ERROR(READ)
        if(n <= 0){
          fprintf(stdout, "~Client [%d] deconnected\n", fds[i].fd);
          close(fds[i].fd);

          //SWAP
          fds[i]  = fds[nfds - 1];
          lens[i] = lens[nfds -1];
          memecpy(buf[i], buf[nfds -1], 1024);
          nfds--;
          i--;

        }

        //CLIENT SPEAK
        else{

          len[i] += n;

          int start = 0;
          for(int j = 0; j < len[i]; j++){
            if(buf[i][j] == '\n'){
              buf[i][j] = '\0';
              fprintf(stdout, "Client [%d]: %s\n", fds[i].fd, &buf[i][start]);
              start = j + 1; 
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
