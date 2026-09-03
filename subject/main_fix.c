#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct t_client
{
	int id;
	char msg[100000];
} s_client;

s_client clients[1024];
int max;
int next_id = 0;
fd_set fd_on;
fd_set fd_r;
fd_set fd_w;
char buf_r[110000];
char buf_w[110000];

void	error() {
	write(2, "Fatal Error\n", 12);
	exit(1);
}

void	send_to_all(int sender)
{
	for (int i =0 ; i <= max; i++)
	{
		if (i != sender && FD_ISSET(i, &fd_w))
			send(i, buf_w, strlen(buf_w), 0);
	}
}

int main(int argc, char **argv)
{

	int	sockfd; // done
	int	connfd; // done
	unsigned int		len; // done
	struct sockaddr_in	servaddr; // done
	struct sockaddr_in	cli; // done

	if (argc != 2) {
		write(2, "Wrong number of arguments\n", 26);
		return (1);
	}

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // done
	if (sockfd == -1)
		error();
	bzero(&servaddr, sizeof(servaddr)); // done

	// assign IP, PORT
	servaddr.sin_family = AF_INET; // done
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(argv[1])); // done

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) // done
		error();
	if (listen(sockfd, 10) != 0) // done
		error();



		FD_ZERO(&fd_on);
	max = sockfd;
	FD_SET(sockfd, &fd_on);
	bzero(&clients, sizeof(clients));

	///////////////////////////HASTA AQUI /////////////////////
	while (1)
	{
		fd_r = fd_w = fd_on;
		if (select(max +1, &fd_r, &fd_w, NULL, NULL) < 0)
			continue;
		for (int s = 0; s <= max; s++)
		{
			if (FD_ISSET(s, &fd_r))
			{
				if (s == sockfd)
				{
					len = sizeof(cli);
					connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
					if (connfd < 0)
						error();
					FD_SET(connfd, &fd_on);
					max = (connfd > max) ? connfd : max;
					clients[connfd].id = next_id++;
					sprintf(buf_w, "server: client %d just arrived\n", clients[connfd].id);
					send_to_all(connfd);
					break;
				}
				else
				{
					int res = recv(s, buf_r, 110000, 0);
					if (res <= 0)
					{
						FD_CLR(s, &fd_on);
						close(s);
						sprintf(buf_w, "server: client %d just left\n", clients[s].id);
						send_to_all(s);
					}
					else
					{
						for (int i = 0, j = strlen(clients[s].msg); i < res; i++, j++)
						{
							clients[s].msg[j] = buf_r[i];
							if (buf_r[i] == '\n')
							{
								clients[s].msg[j] = '\0';
								sprintf(buf_w, "client %d: %s\n", clients[s].id, clients[s].msg);
								send_to_all(s);
								bzero(&clients[s].msg, sizeof(clients[s].msg));
								j = -1;
							}
						}
					}
					break;
				}
			}
		}
	}
}