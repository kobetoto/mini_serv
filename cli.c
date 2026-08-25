#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){

  int   fd;
  struct sockaddr_in addr;

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd == -1){perror("socket"); return (1);}

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1 ){
    perror("connect"); return (1);
  }
  fprintf(stdout, "connect to the server: success!\n");
  
  sleep(30);

  write(fd, "hello from cli\n", 15);

  char buf[1024];

  ssize_t n = read(fd, buf, sizeof(buf) -1);
  if(n > 0){
    buf[n] = '\0';
    fprintf(stdout, "...server:\n%s\n", buf);
  }
  close(fd);
  return (0);
}
