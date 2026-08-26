#include <netinet/in.h>
#include <poll.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 * struct pollfd{
 *      int fd; 
 *      short events;
 *      short revents;
 * }
 * */

int main(void){

  int fd_serv;

  fd_serv = socket(AF_INET, SOCK_STREAM, 0);
  if(fd_serv == -1){
    perror("socket"); return (1);}

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  addr.sin_addr.s_addr = INADDR_ANY;

  if(bind(fd_serv, (const struct sockaddr*) &addr,sizeof(addr)) == -1){
    perror("bind"); return (1);}

  if(listen(fd_serv, 42) == -1){
    perror("bind"); return (1);}
  fprintf(stdout,"Server is live...\n");

  struct pollfd fds[142];
  fds[0].fd = fd_serv;
  fds[0].events = POLLIN;

  int nfds = 1;

  for(;;){

    if(poll(fds, nfds, -1) == -1){
      perror("poll"); return (1);}

    for(int i = 0; i < nfds; i++){

      if(!(fds[i].revents & POLLIN))
        continue;

      if(fds[i].fd == fd_serv){
        int fd_cli = accept(fd_serv, NULL, NULL);
        if(fd_cli == -1){perror("accept"); continue;}
        fds[nfds].fd = fd_cli;
        fds[nfds].events = POLLIN;
        nfds ++;
        fprintf(stdout,"new client fd: %d\n %d clients in the server\n", fd_cli, nfds);
      }
      else{
        char buf[1024];
        ssize_t n = read(fds[i].fd, &buf, sizeof(buf) -1);
        if(n > 0){
          buf[n] = '\0';
          fprintf(stdout,"Client [%d]:\n%s\n", fds[i].fd, buf);
          write(fds[i].fd, "Server:\nRecu 5/5\n", 18);
        }
        else{
          if(n == 0){fprintf(stdout,"Client [%d] exit the server!\n",fds[i].fd);}
          else{perror("read");}
          close(fds[i].fd);
          fds[i] = fds[nfds-1];
          nfds--;
          i--;
        }
      }
    }
  }
  return (0);
}
