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

void parse_header(int con_sock, int *num_threads, int *file_size){
	int i = 0;
	char c_num_threads[20], c_file_size[20];
	char c[1];
	//primeiro recebo o número de threads
	int bytesRcvd = recv(con_sock, c, 1, 0);
	while(bytesRcvd && c[0] != '\n'){
		if(bytesRcvd == -1){
			fprintf(stderr, "Deu merda!\n");
			exit(1);
		}
		c_num_threads[i] = c[0];
		i++;
		bytesRcvd = recv(con_sock, c, 1, 0);
	}
	*num_threads = atoi(c_num_threads);

	//depois o tamanho do arquivo
	i = 0;
	bytesRcvd = recv(con_sock, c, 1, 0);
	while(bytesRcvd && c[0] != '\n'){
		if(bytesRcvd == -1){
			fprintf(stderr, "Deu merda!\n");
			exit(1);
		}
		c_file_size[i] = c[0];
		i++;
		bytesRcvd = recv(con_sock, c, 1, 0);
	}
	*file_size = atoi(c_file_size);

	//Só retiro o 2 \ns para escrever corretamente no arquivo
	bytesRcvd = recv(con_sock, c, 1, 0);
}

/**
 *	Lê uma linha completa de um arquivo
 **/
void read_line(int fd, char conent[]){
	char c[1];
	int i = 0;
	int bytes_read = read(fd, c, 1);
	while(bytes_read > 0 && c[0] != '\n'){
		conent[i] = c[0];
		bytes_read = read(fd, c, 1);
		i++;
	}
	conent[i] = '\n';
}
/**
 *	Vai ler as 2 primeiras linhas respectivas à sua thread para pegar o offset do arquivo e o tamanho do segmento
 **/
void server_thread_params(int con_sock, int *offset, int *segment_size){
	char c_offset[30], c_segment_size[30];
	read_line(con_sock, c_offset);
	read_line(con_sock, c_segment_size);
	*offset = atoi(c_offset);
	*segment_size = atoi(c_segment_size);
}
