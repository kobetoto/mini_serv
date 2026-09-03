#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
struct pollfd fds[100];
int nfds = 0;
char *buf[100000];
int ids[100];
int id_next = 0;
char box[10000];
char tmp[10000];
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
void error(){
	write(2,"Fatal error\n", 12);
	exit(1);
}
void  broadcast(int skip, char *msg, int len){
	for(int i = 1; i< nfds; i++)
		if(i != skip)
			send(fds[i].fd, msg, len, 0);
}
int main(int ac, char **av) {
	if(ac != 2){
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}
	int sockfd;
	struct sockaddr_in servaddr; 
	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) 
		error();	
	bzero(&servaddr, sizeof(servaddr)); 
	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1])); 
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) == -1) { 
		close(sockfd);
		error();
	} 
	if (listen(sockfd, 10) == -1) {
		close(sockfd);
		error();
	}
	fds[0].fd = sockfd;
	fds[0].events = POLLIN;
	fds[0].revents = 0;
	++nfds;




	for(;;){


		if(poll(fds, nfds, -1) == -1){
			close(sockfd);
			error();
		}

		for(int i = 0; i < nfds;  i++){
			if(!(fds[i].revents & POLLIN))
				continue;
			if(fds[i].fd == sockfd){
				int sockCli = accept(sockfd, NULL, NULL);
				if(sockCli == -1)
					continue;
				if(nfds >= 100){
					close(sockCli);
					continue;
				}
				fds[nfds].fd = sockCli;
				fds[nfds].events = POLLIN;
				fds[nfds].revents = 0;
				ids[nfds] = id_next;
				buf[nfds] = NULL;
				++id_next;
				++nfds;
				int len = sprintf(box, "server: client %d just arrived\n", ids[nfds -1]);
				broadcast(nfds-1, box, len);
			}




			else{
				ssize_t n = recv(fds[i].fd, tmp, 9999, 0);
				if(n <= 0){
					int len = sprintf(box, "server: client %d just left\n", ids[i]);
					broadcast(i, box, len);
					//swaaap
					close(fds[i].fd);
					free(buf[i]);
					fds[i] = fds[nfds -1];
					ids[i] = ids[nfds -1];
					buf[i] = buf[nfds -1];
					--nfds;
					--i;
				}



				else{
					tmp[n] = 0; //car tmp[len] recv(,,len-1,)
					buf[i] = str_join(buf[i], tmp);
					if(!buf[i]){error();}
					char *msg;
					int r;
					while((r = extract_message(&buf[i], &msg)) == 1){
						int len = sprintf(box, "client %d: %s", ids[i], msg);
						broadcast(i, box, len);
						free(msg);
					}
					if(r == -1){
						close(sockfd);
						error();	
					}
				}
			}
		}	
	}
	return (0);
}

