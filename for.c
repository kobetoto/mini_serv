//global
struct pollfd fds[100];
int nfds = 0;
char *buf[100];
int ids[100];
int id_next = 0;
char box[100000];
char tmp[100000];

void broadcast(int skip, char *msg, int len){
  for(int i = 1; i < nfds; i++)
    if(i != skip)
      send( fds[i].fd, msg, len, 0);
}


//[...]

fds[0].fd = sockfd;
fds[0].events = POLLIN;
fds[0].revents = 0;

for(;;){

  if(poll() == -1){
    close(sockfd);
    error();
  }

  for(int i = 0; i < nfds; i++){

    if(!(fds[i].revents & POLLIN))
      continue;

    if(fds[i].fd == sockfd){
      int sockcli = accept(sockfd, NULL, NULL);
      if(sockcli == -1)
        continue;
      if(nfds >= 100){
        close(sockcli);
        continue;
      }

      fds[nfds].fd = sockcli;
      fds[nfds].events = POLLIN;
      fds[nfds].revents = 0;
      ids[nfds] = id_next;
      buf[nfds] = NULL;
      ++id_next;
      ++nfds;

      int len = sprintf(box, "client %d just arrived\n", ids[nfds -1]);
      broadcast(nfds -1, box, len);
    }
    else{
      ssize_t n = recv(fds[i].fd,tmp, 99999, 0);

      if( n <= 0){

        int len = sprintf(box, "client %d just left\n", ids[i]);
        broadcast(i, box, len);       

        //swap
        close(fds[i].fd);
        free(buf[i]);
        fds[i] = fds[nfds -1];
        ids[i] = ids[nfds -1];
        buf[i]  = buf[nfds -1];
        --i;
        --nfds;
      }
      else{
        tmp[n] = '\0';
        buf[i] = strjoin(buf[i], tmp);
        if(!buf[i])error();
        char *msg;
        int r;
        while((r = extract_message(&buf[i],&msg)) == 1){ 
        int len = sprintf(box, "client %d: %s", ids[i], msg);
        broadcast(i, box, len);       
        free(msg);
        }
        if(r == -1) error();
      }
    }
  }
}
