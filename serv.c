#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(){

  int fd;
  struct sockaddr_in addr;

  fd = socket(AF_INET, SOCK_STREAM,0);
  if (fd == -1){
    perror("socket"); return 1;
  }


  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1){
    perror("bind"); return 1;}
    
  if(listen(fd, 10) == -1){
    perror("listen"); return 1;}

  fprintf(stdout, "server run sur:: 0.0.0.0:8080 (fd=%i)\n", fd);

  int fd_client = accept(fd, NULL, NULL);
  if(fd_client == -1){
    perror("listen"); return 1;}
  
  fprintf(stdout, "Client accepte!!!!!\n");

  char buf [1024];

  ssize_t n_read = read(fd_client, buf, sizeof(buf) -1);
  if (n_read > 0){
      fprintf(stdout, "data recu = %zd\n\n%s\n", n_read, buf);
      write(fd_client, "bien recu\n", 11);
  }

  close(fd_client);
  close(fd);
return (0);
}
