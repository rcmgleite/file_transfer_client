/*
 * client_utils.c
 *
 *  Created on: Jun 30, 2014
 *      Author: rafael
 */
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#include "client_utils.h"

int create_connection(char *host, char *port){
	fprintf(stdout, "host: %s\n", host);
	fprintf(stdout, "port: %s\n", port);
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in si;
	memset(&si, 0, sizeof(si));
	si.sin_family = PF_INET;
	si.sin_addr.s_addr = inet_addr(host);
	si.sin_port = htons(atoi(port));
	fprintf(stdout, "Antes do 'connect'\n");
	int c = connect(sock, (struct sockaddr*)&si, sizeof(si));
	if(c == -1){
		fprintf(stdout, "falha na conexão\n");
		exit(1);
	}
	else{
		fprintf(stdout, "Conectou bala!\n");
	}
	return sock;
}
