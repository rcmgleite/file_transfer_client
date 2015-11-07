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

char *build_file_path(char* file_name, char *dir_path){
	size_t i, slen = strlen(file_name);
	for (i = 0; i < slen; i++) {
		if(file_name[i] == '\r')
			file_name[i] = '\0';
	}
	slen = strlen(dir_path);
	int prog_name_size = strlen(PROGRAM_NAME);
	char *corrected_dir_path = (char*) malloc((slen + 1 - prog_name_size) * sizeof(char*));

	i = 0;
	while(i < slen - strlen(PROGRAM_NAME)){
		corrected_dir_path[i] = dir_path[i];
		i++;
	}

	char *file_path;
	file_path = malloc(strlen(corrected_dir_path) + strlen(file_name));
	strcpy(file_path, corrected_dir_path);
	strcat(file_path, file_name);
	return file_path;
}

int create_connection(char *host, char *port){
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	/**
	 *	sockaddr_in é uma variação de sockaddr para ser usada na internet(in)
	 *	A função 'connect()' espera um sockaddr, portanto devemos fazer o cast de
	 *	sockaddr_in para sockaddr na hora da chamada
	 **/
	struct sockaddr_in si;
	memset(&si, 0, sizeof(si));
	si.sin_family = PF_INET;
	/**
	 *	A função inet_addr já está obsoleta e não funciona com ipv6...
	 **/
	//	si.sin_addr.s_addr = inet_addr(host);
	inet_pton(AF_INET, host, &(si.sin_addr));
	si.sin_port = htons(atoi(port));
	int c = connect(sock, (struct sockaddr*)&si, sizeof(si));
	/**
	 *	Spin_lock para conectar
	 **/
	int i = 0;
	while(c == -1 && i < MAX_ITER){
		fprintf(stdout, "falha na conexão com a porta %s\n Tentando novamente...\n", port);
		c = connect(sock, (struct sockaddr*)&si, sizeof(si));
	}
	return sock;
}

void parse_header(int con_sock, int *num_threads, long *file_size){
	int i = 0;
	char c_num_threads[255], c_file_size[255];
	char c[1];
	//primeiro recebo o número de threads
	int bytesRcvd = recv(con_sock, c, 1, 0);
	while(bytesRcvd && c[0] != '\n'){
		if(bytesRcvd == -1){
			fprintf(stderr, "Erro em client_utils.c na função parse_header!\n");
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
			fprintf(stderr, "Erro em client_utils.c na função parse_header!\n");
			exit(1);
		}
		c_file_size[i] = c[0];
		i++;
		bytesRcvd = recv(con_sock, c, 1, 0);
	}
	*file_size = atol(c_file_size);
	//Só retiro o 2 \ns para escrever corretamente no arquivo
	//	bytesRcvd = recv(con_sock, c, 1, 0);
}

/**
 *	Lê uma linha completa de um arquivo
 **/
void read_line(int fd, char conent[255]){
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
void server_thread_params(int con_sock, long *offset, long *segment_size){
	char c_offset[255], c_segment_size[255];
	read_line(con_sock, c_offset);
	read_line(con_sock, c_segment_size);
	*offset = atol(c_offset);
	*segment_size = atol(c_segment_size);
}

void wait_init(int con_sock){
	char init[5];
	recv(con_sock, init, 5, 0);
}
