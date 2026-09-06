else{
	tmp[n] = 0;
	buf[i] = str_join(buf[i], tmp);
	if(!buf[i]) error();
	char *msg;
	int r;
	while((r = extract_message(&buf[i], &msg)) == 1){
		int len = sprintf(box, "client %i: %s", ids[i]), msg;
		broadcast(i, box, len);
		free(msg);
	}
	if(r == -1) error();
}
