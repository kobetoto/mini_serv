#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


int main(){

  int             socket_serv;
  struct sockaddr_in addr;

  socket_serv = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_serv == -1){perror("socket"); return (1);}

  memset(&addr, 0, sizeof(addr));

  addr.sin_port = htons(8080);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;

  if( (bind(socket_serv, (const struct sockaddr *)&addr, sizeof(addr))) == -1 )
  {perror("bind"); return (1);}

  if(listen(socket_serv, 10) == -1){perror("listen"); return (1);}

  fprintf(stdout, "Socket, bind, listen: OK\n");

  struct pollfd fds[100];
  int nfds = 1;

  fds[0].fd = socket_serv;
  fds[0].events = POLLIN;

  for(;;){

    if(poll(fds, nfds, -1) == -1){perror("poll"); return (1);}

    for(int i = 0; i < nfds; i++){

      if(!(fds[i].revents & POLLIN))
        continue;

      if(fds[i].fd == socket_serv){

        int s_client = accept(socket_serv, NULL, NULL);
        if(s_client == -1){perror("accept"); continue;}
        fds[nfds].fd = s_client;
        fds[nfds].events = POLLIN;
        nfds++;
        fprintf(stdout,"New client: fd->%d\nNumberOfClient: %d\n",s_client, nfds);
      }
      else{
        char buf[1024];
        ssize_t n = read(fds[i].fd, &buf, sizeof(buf) - 1);
        if(n > 0){
          buf[n] = '\0';
          fprintf(stdout, "Client [%d]:\n->%s\n",fds[i].fd, buf);
          write(fds[i].fd, "Serv: OK\n", 9);
        }
        else{
          if(n == 0){
            fprintf(stdout,"Client [%d] left\n", fds[i].fd);
          }
          else{
            perror("read");
          }
          close(fds[i].fd);
          fds[i] = fds[nfds - 1];
          nfds --;
          i--;
        }
      }
    }
  }
  return (0);
}
