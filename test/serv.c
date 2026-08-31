#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

/*
struct pollfd {
  int   fd;         // file descriptor
  short events;     // requested events
  short revents;    // returned events 
};

*/

int main(){

  int fd_serv;
  struct sockaddr_in addr;

  fd_serv = socket(AF_INET, SOCK_STREAM,0);
  if (fd_serv == -1){
    perror("socket"); return 1;
  }


  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(fd_serv, (struct sockaddr *)&addr, sizeof(addr)) == -1){
    perror("bind"); return 1;}

  if(listen(fd_serv, 10) == -1){
    perror("listen"); return 1;}

  fprintf(stdout, "server run sur:: 0.0.0.0:8080 (fd=%i)\n", fd_serv);

  struct pollfd fds[128];
  fds[0].fd = fd_serv;
  fds[0].events = POLLIN;
  int nfds = 1;

  for(;;){

    if(poll(fds, nfds, -1) == -1){
      perror("poll"); return 1;}

    for(int i = 0; i < nfds; i++){
      fprintf(stdout,"debug\n");
      if (!(fds[i].revents & POLLIN))
        continue;
      if(fds[i] == fd_serv){

        int fd_client = accept(fd_serv, NULL, NULL);
        if(fd_client == -1){
          perror("listen"); return 1;}

        fprintf(stdout, "Client accepte!!!!!\n");
        }
        else{

        char buf [1024];

        ssize_t n_read = read(fd_client, buf, sizeof(buf) -1);
        if (n_read > 0){
          fprintf(stdout, "data recu = %zd\n\n%s\n", n_read, buf);
          write(fd_client, "bien recu\n", 11);
        }
        if(n_read == 0){...}
        if(n_read == -1){perror("poll"); close(fd_serv); close(fd_client); return (1)}
      }
    }
    close(fd_serv);
    return (0);
  }
