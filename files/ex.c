#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}


struct pollfd fds[100];
char         *buf[100];        /* CHANGÉ : char buf[100][100000] → char *buf[100] (nés NULL) */
int           ids[100];
int           next_id = 0;
int           nfds = 0;
char          box[200000];
char          tmp[65536];      /* CHANGÉ : la pelle du recv (nouveau, remplace lens) */
/* CHANGÉ : lens[100] SUPPRIMÉ — l'école-malloc n'en a plus besoin */

void broadcast(int skip, char *msg, int len){   /* CHANGÉ : fds/nfds globaux → 3 params */
  for(int i = 1; i < nfds; i++)
    if(i != skip)
      send(fds[i].fd, msg, len, 0);
}
void error(){
  write(2,"Fatal error\n", 12);
  exit(1);
}

int main(int ac, char **av){
  if(ac != 2){
    write(2, "Wrong number of arguments\n", 26);   /* (ta version traversée disait error() — le sujet veut CE message) */
    exit(1);
  }


  int socket_serv = socket(AF_INET, SOCK_STREAM,0);
  if(socket_serv == -1)
    error();
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(av[1]));
  addr.sin_addr.s_addr = htonl(2130706433);
  if (bind(socket_serv, (const struct sockaddr *)&addr, sizeof(addr)) == -1){
    close(socket_serv);
    error();
  }
  if(listen(socket_serv, 42) == -1){
    close(socket_serv);
    error();
  }


  //part2 — carnet : INCHANGÉE
  fds[0].fd = socket_serv;
  fds[0].events = POLLIN;
  fds[0].revents = 0;
  nfds++;

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
        if(nfds >= 100){ close(socket_cli); continue; }
        fds[nfds].fd = socket_cli;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        buf[nfds] = NULL;                 /* CHANGÉ : lens[nfds]=0 → buf[nfds]=NULL */
        ids[nfds] = next_id;
        next_id++;
        nfds++;
        int len = sprintf(box, "server: client %d just arrived\n", ids[nfds -1]);
        broadcast(nfds-1, box, len);
      }



      else{
        ssize_t n = recv(fds[i].fd, tmp, 65535, 0);   /* CHANGÉ : la pelle, plus &buf[i][lens[i]] */
        
        if(n <= 0){
          int len = sprintf(box, "server: client %d just left\n", ids[i]);
          broadcast(i, box, len);
          close(fds[i].fd);
          free(buf[i]);                   /* CHANGÉ : libérer sa feuille */
          fds[i] = fds[nfds-1];
          buf[i] = buf[nfds-1];           /* CHANGÉ : swap du POINTEUR (memcpy 100000 supprimé) */
          ids[i] = ids[nfds-1];
          --nfds;
          --i;
        }

        else{
          /* CHANGÉ : tout le bloc start/memmove → le trio sujet */
          tmp[n] = '\0';
          buf[i] = str_join(buf[i], tmp);
          if(!buf[i]) error();
          char *msg;
          int r;
          while((r = extract_message(&buf[i], &msg)) == 1){
            int len = sprintf(box, "client %d: %s", ids[i], msg);  /* PAS de \n : msg l'a */
            broadcast(i, box, len);
            free(msg);
          }
          if(r == -1) error();
        }
      }
    }
  }
  return (0);
}
