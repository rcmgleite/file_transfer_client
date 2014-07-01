/*
 * ftp_client.c
 *
 *  Created on: Jun 30, 2014
 *      Author: rafael
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "ftp_client.h"
#include "client_utils.h"

int main(int argc, char *argv[]){
	argc = 3;
	argv[1] = "127.0.0.1";
	argv[2] = "30000";

	if(argc != 3){
		fprintf(stderr, "Passa os argumentos direito, porra\n");
		exit(1);
	}

	fprintf(stdout, "Vai criar a coneção!\n");
	int con_sock = create_connection(argv[1], argv[2]);
	fprintf(stdout, "Criou a conexão!\n");
	char to_say[] = "teste.txt";
	char rec[1024];

	if(recv(con_sock, rec, 1024, 0) != -1){
		fprintf(stderr, "recebeu: %s\n", rec);
		send(con_sock, to_say, sizeof(to_say), 0);
	}
	else{
		fprintf(stderr, "Cagou tudo aqui na hora de ler o primeiro trecho do protocolo...\n");
	}
	fprintf(stderr, "enviou o nome do arquivo: %s\n", to_say);

	/**
	 * 	Abro o arquivo que vou escever
	 **/
	int fd_to_write = open("/home/rafael/Desktop/rafael/C/proj_redes_client/Debug/teste_new.txt",O_RDWR | O_CREAT, S_IRUSR|S_IWUSR);

	char teste[1];

	int bytesRcvd = recv(con_sock, teste, 1, 0);
	while(bytesRcvd){
		if(bytesRcvd == -1){
			fprintf(stderr, "Deu merda!\n");
			exit(1);
		}
		fprintf(stderr, "%c", teste[0]);
		write(fd_to_write, teste, sizeof(teste));
		bytesRcvd = recv(con_sock, teste, 1, 0);
	}
	close(con_sock);

	return 0;
}
