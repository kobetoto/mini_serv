#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

int main(void){
  int fd_serv = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_serv == -1){ perror("socket"); return 1; }

  int yes = 1;
  setsockopt(fd_serv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(fd_serv, (struct sockaddr *)&addr, sizeof(addr)) == -1){
    perror("bind"); return 1; }
  if (listen(fd_serv, 10) == -1){ perror("listen"); return 1; }
  printf("serveur poll sur 0.0.0.0:8080\n");

  struct pollfd fds[128];              /* LE CARNET */
  fds[0].fd = fd_serv;                 /* ligne 0 : l'accueil, à vie */
  fds[0].events = POLLIN;
  int nfds = 1;                        /* lignes remplies du carnet */

  for (;;){
    if (poll(fds, nfds, -1) == -1){ perror("poll"); return 1; }

    for (int i = 0; i < nfds; i++){
      if (!(fds[i].revents & POLLIN))
        continue;                      /* cette ligne n'a pas sonné */

      if (fds[i].fd == fd_serv){       /* SONNETTE ACCUEIL : on frappe */
        int fd_client = accept(fd_serv, NULL, NULL);
        if (fd_client == -1){ perror("accept"); continue; }
        fds[nfds].fd = fd_client;      /* ← L'INSCRIPTION AU CARNET : */
        fds[nfds].events = POLLIN;     /*   les 3 lignes qui manquaient */
        nfds++;
        printf("client inscrit (fd=%d) — %d fd surveillés\n", fd_client, nfds);
      }
      else {                           /* SONNETTE SALON : il a parlé */
        char buf[1024];
        ssize_t n = read(fds[i].fd, buf, sizeof(buf) - 1);  /* le fd VIENT du carnet */
        if (n > 0){
          buf[n] = '\0';
          printf("fd=%d dit : %s", fds[i].fd, buf);
          write(fds[i].fd, "bien recu\n", 10);
        }
        else {                         /* n==0 (raccroché) ou -1 (erreur) */
          if (n == 0) printf("fd=%d a raccroche\n", fds[i].fd);
          else perror("read");
          close(fds[i].fd);            /* fermer le salon */
          fds[i] = fds[nfds - 1];      /* la dernière ligne comble le trou */
          nfds--;                      /* le carnet rétrécit */
          i--;                         /* re-examiner la ligne déplacée ! */
        }
      }
    }
  }
}
