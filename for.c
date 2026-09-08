
void 	broadcast(int skip, char *msg, int len){
	for(int i = 1; i < nfds; i++)
		if(i != skip && (fds[i].revents & POLLOUT))
			send(fds[i].fd, msg, len, 0);
}

for(;;){

	if(poll(fds, nfds, -1) == -1) error();

	for(int i = 0; i < nfds; i++){

		if(!(fds[i].revents & POLLIN))
			continue;
		if(fds[i].fd == sockfd){
			int sockCli = accept(sockfd, NULL, NULL);
			if(sockCli == -1){ continue; }
			if(nfds >= 100){ close(sockCli); continue; }

			fds[nfds].fd = sockCli;
			fds[nfds].events = POLLIN | POLLOUT;
			fds[nfds].revents = 0;
			buf[nfds] = NULL;
			ids[nfds] = next_id;
			++next_id;
			++nfds;

			int len = sprintf(box, "server: client %d just arrived\n", ids[nfds-1]);
			broadcast(nfds-1, box, len);
		}
		else{
			ssize_t n = recv(fds[i].fd, tmp, 99999, 0);
			if(n <= 0){
				int len = sprintf(box, "server: client %d just left\n", ids[i]);
				broadcast(i, box, len);

				close(fds[i].fd);
				free(buf[i]);
				fds[i] = fds[nfds-1];
				ids[i] = ids[nfds-1];
				buf[i] = buf[nfds-1];
				--i;
				--nfds;
			}
			else{
				tmp[n] = 0;
				buf[i] = str_join(buf[i], tmp);
				if(!buf[i]) error();
				char *msg;
				int r;
				while((r = extract_message(&buf[i], &msg)) == 1){
					int len = sprintf(box, "client %d: %s", ids[i], msg);
					broadcast(i, box, len);
					free(msg);
				}
				if(r == -1) error();
			}
		}
	}
}
