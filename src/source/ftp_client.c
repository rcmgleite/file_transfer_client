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
#include <fcntl.h>
#include <pthread.h>

#include "ftp_client.h"
#include "client_utils.h"

pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]){
	//TODO remover
	argc = 3;
	argv[1] = "127.0.0.1";
	argv[2] = "30000";

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
	int num_threads, file_size;

	/**
	 * 	Nos headers eu tenho o número de threads e o tamanho do arquivo que vai ser recebido
	 **/
	parse_header(con_sock, &num_threads, &file_size);

	/**
	 *	Daqui pra frente lê o que o servidor manda e monta o arquivo final
	 **/

	/**
	 *	Alocação de memória para threads e para a estrutura de argumentos
	 **/
	pthread_t *threads;
	threads = (pthread_t *)malloc(num_threads * sizeof(*threads));
	struct thread_args *args;
	args = (struct thread_args *)malloc(num_threads * sizeof(*args));

	/**
	 *	Inicialização das threads
	 **/
	int i;
	for(i = 0; i < num_threads; i++){
		initialize_thread(&threads[i], &args[i], i, con_sock, fd_to_write, &file_size, &curr_offset, fd_to_write);
	}

	//Apenas para o programa esperar as threads executarem
	for (i = 0; i < num_threads; i++){
		pthread_join(threads[i], NULL);
	}

	// Limpo todos os dados para a próxima requesição
	clean_up(fd_to_write, threads, args, &num_threads, &curr_offset, &file_size);



	char c[1];
	int bytesRcvd = recv(con_sock, c, 1, 0);
	while(bytesRcvd){
		if(bytesRcvd == -1){
			fprintf(stderr, "Deu merda!\n");
			exit(1);
		}
		fprintf(stderr, "%c", c[0]);
		write(fd_to_write, c, sizeof(c));
		bytesRcvd = recv(con_sock, c, 1, 0);
	}
	close(con_sock);

	return 0;
}

void *thread_function(void *args){
	pthread_mutex_lock(&_lock);
	char *file_segment;
	file_segment = malloc(((_thread_args*)args)->chunk_size * sizeof(*file_segment));
	int bytes_read;
	fprintf(stdout, "\n\nSERÃO LIDOS: %d\n", ((_thread_args*)args)->chunk_size);
	fprintf(stdout, "\nTHREAD NUMBER: %d\n", ((_thread_args*)args)->thread_number);
	lseek(((_thread_args*)args)->fd_to_write, ((_thread_args*)args)->file_offset, SEEK_SET);

	bytes_read = read(((_thread_args*)args)->server_sock, file_segment, ((_thread_args*)args)->chunk_size);
	if(bytes_read < 0)
		fprintf(stderr, "\nErro ao tentar ler arquivo pedido\n\n");
	fprintf(stdout, "%s\n\n", file_segment);
	lseek(((_thread_args*)args)->fd_to_write, ((_thread_args*)args)->file_offset, SEEK_SET);
	write(((_thread_args*)args)->fd_to_write, file_segment, ((_thread_args*)args)->chunk_size);
	free(file_segment);
	pthread_mutex_unlock(&_lock);
	return NULL;
}

void initialize_thread(pthread_t *thread, struct thread_args *args, int thread_number, int server_sock, int fd_to_write, int *file_size, int *curr_offset){
	args->thread_number = thread_number;
	args->fd_to_write = fd_to_write;
	//parte da incialização que muda para cada thread
	if(*file_size - FIRST_GUESS_OFFSET > 0){
		//ainda está no ponto onde as threads escrevem exatamente o tamanho do chunk
		args->file_offset = *curr_offset;
		args->chunk_size = FIRST_GUESS_OFFSET;
		*file_size -= args->chunk_size;
		*curr_offset +=args->chunk_size;
	}
	else{
		//aqui já teremos um valor menor de chunk (última parte do arquivo)
		args->file_offset = *curr_offset;
		args->chunk_size = *file_size;
		*file_size -= args->chunk_size;
		*curr_offset +=args->chunk_size;
	}
	pthread_create(thread, NULL, thread_function, (void*)args);
}

void clean_up(int fd_to_write, pthread_t *threads, struct thread_args *args, int *number_of_threads,
		int *file_size, int *curr_offset){
    close(fd_to_write);
    free(threads);
    free(args);
	*number_of_threads = 0;
	*file_size = 0;
	*curr_offset = 0;
}
