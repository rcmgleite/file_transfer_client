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
#include <pthread.h>

#include "ftp_client.h"
#include "client_utils.h"

pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]){
	//TODO remover
//	argc = 3;
//	argv[1] = "127.0.0.1";
//	argv[2] = "30000";

	if(argc != 3){
		fprintf(stderr, "Passa os argumentos direito, porra\n");
		exit(1);
	}

	/**
	 *	Variáveis necessárias
	 **/
	int curr_offset = 0;

	fprintf(stdout, "Vai criar a coneção!\n");
	int con_sock = create_connection(argv[1], argv[2]);
	fprintf(stdout, "Criou a conexão!\n");
	char to_say[255];
	char rec[1024];

	scanf("%s", to_say);

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
	char *path_to_write = build_file_path(to_say);
	fprintf(stderr, "caminho do arquivo transferido: %s\n", path_to_write);
	int fd_to_write = open(path_to_write ,O_RDWR | O_CREAT, S_IRUSR|S_IWUSR);
	if(fd_to_write == 1){
		fprintf(stderr, "Deu merda pra abrir o arquivo do cliente\n");
		close(con_sock);
		exit(1);
	}
	int num_threads, file_size;

	/**
	 * 	Nos headers eu tenho o número de threads e o tamanho do arquivo que vai ser recebido
	 **/
//	char bb[1024];
//	recv(con_sock, bb, 1024, 0);
	fprintf(stderr, "Antes de fazer o parse do Header\n");
	parse_header(con_sock, &num_threads, &file_size);
	fprintf(stderr, "Depois de fazer o parse do Header\n");

	/**
	 *	Daqui pra frente lê o que o servidor manda e monta o arquivo final
	 *
	 *	Alocação de memória para threads e para a estrutura de argumentos
	 **/
	pthread_t *threads;
	threads = (pthread_t *)malloc(num_threads * sizeof(*threads));
//	struct thread_args *args;
//	args = (struct thread_args *)malloc(num_threads * sizeof(*args));

	/**
	 *	Inicialização das threads
	 **/
	int i;
	for(i = 0; i < num_threads; i++){
		struct thread_args *args;
		args = (struct thread_args *)malloc(sizeof(*args));
		initialize_thread(&threads[i], args, i, con_sock, fd_to_write);
	}

	//Apenas para o programa esperar as threads executarem
	for (i = 0; i < num_threads; i++){
		pthread_join(threads[i], NULL);
	}

	// Limpo todos os dados para a próxima requesição
	clean_up(fd_to_write, threads, &num_threads, &curr_offset, &file_size);

	close(con_sock);

	return 0;
}

void *thread_function(void *args){
	pthread_mutex_lock(&_lock);

	/**
	 *	Lê o offset e o segment_size que o servidor enviar para remontar o arquivo
	 **/
	int offset, segment_size;
	server_thread_params(((_thread_args*)args)->server_sock, &offset, &segment_size);

	char *file_segment;
	file_segment = (char*) malloc(segment_size * sizeof(*file_segment));
	int bytes_read;
	fprintf(stdout, "\n\nSERÃO LIDOS: %d\n", segment_size);
	fprintf(stdout, "\nTHREAD NUMBER: %d\n", ((_thread_args*)args)->thread_number);

	/**
	 *	Leitura do socket que vai conter o contéudo do arquivo
	 **/
	bytes_read = read(((_thread_args*)args)->server_sock, file_segment, segment_size);
	if(bytes_read < 0)
		fprintf(stderr, "\nErro ao tentar ler arquivo pedido\n\n");
	/**
	 *	lseek vai mudar o ponteiro do arquivo para escrever no local correto
	 **/
	lseek(((_thread_args*)args)->fd_to_write, offset, SEEK_SET);
	write(((_thread_args*)args)->fd_to_write, file_segment, segment_size);

	/**
	 *	Libero a memória do file_segment
	 **/
	free(file_segment);
	free(args);
	pthread_mutex_unlock(&_lock);
	return NULL;
}

void initialize_thread(pthread_t *thread, struct thread_args *args, int thread_number, int server_sock, int fd_to_write){
	args->thread_number = thread_number;
	args->fd_to_write = fd_to_write;
	args->server_sock = server_sock;
	pthread_create(thread, NULL, thread_function, (void*)args);
}

void clean_up(int fd_to_write, pthread_t *threads, int *number_of_threads,
		int *file_size, int *curr_offset){
    close(fd_to_write);
    free(threads);
	*number_of_threads = 0;
	*file_size = 0;
	*curr_offset = 0;
}
