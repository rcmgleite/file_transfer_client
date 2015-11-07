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
#include <sys/resource.h>
#include <sys/time.h>

#include "ftp_client.h"
#include "client_utils.h"
#include "thread_pool.h"

typedef struct thread_args{
	int server_sock;
	int thread_number;
	char *file_path;
}_thread_args;

static void initialize_thread_args(struct thread_args *args, int thread_number, int server_sock, char *file_path);
static void *thread_function(void *args);

/**
 *	Program params are the same as telnet: ip port
 **/
int main(int argc, char *argv[]){
	if(argc != 3){
		fprintf(stderr, "Passe os parametros 'ip' e 'porta' para que o cliente possa se conectar ao cliente.\n");
		exit(1);
	}

	int con_sock = create_connection(argv[1], argv[2]);
	char to_say[255];
	char rec[1024];

	fprintf(stderr, "Digite o nome do arquivo desejado: ");
	scanf("%s", to_say);

	if(recv(con_sock, rec, 1024, 0) != -1){
		fprintf(stderr, "received: %s\n", rec);
		send(con_sock, to_say, sizeof(to_say), 0);
	}
	else{
		fprintf(stderr, "Error during client initialization\n Shutting down!...\n");
		exit(1);
	}
	fprintf(stderr, "sent file_name: %s\n", to_say);

	char *path_to_write = format_file_path(to_say);
	fprintf(stderr, "path to received file: %s\n", path_to_write);
	int fd_to_write = open(path_to_write ,O_RDWR | O_CREAT, S_IRUSR|S_IWUSR);
	if(fd_to_write == 1){
		fprintf(stderr, "Error while trying to create the new file\n Shutting down");
		close(con_sock);
		exit(1);
	}

	close(fd_to_write);
	int num_threads;
	long file_size;

	/**
	 * 	Headers: n_threads e file size
	 **/
	parse_header(con_sock, &num_threads, &file_size);

	fprintf(stderr, "server num_threads: %d\n", num_threads);

	thread_pool_t* pool = new_thread_pool(num_threads);
	wait_init(con_sock);

	struct timeval tvalBefore, tvalAfter;  // removed comma

	gettimeofday (&tvalBefore, NULL);

	for(unsigned i = 0; i < num_threads; i++){
		struct thread_args *args;
		args = (struct thread_args *)malloc(sizeof(*args));
		initialize_thread_args(args, i, con_sock, path_to_write);
		pool_add_job(pool, thread_function, args);
	}

	pool_wait_finish(pool);

	gettimeofday (&tvalAfter, NULL);

	fprintf(stderr, "Aqruivo recebido!\n");

	printf("Time Elapsed: %f sec\n",
				                ((tvalAfter.tv_sec - tvalBefore.tv_sec)
				               + (tvalAfter.tv_usec - tvalBefore.tv_usec)/(float)1000000)
				              );
	close(con_sock);

	return 0;
}

static void *thread_function(void *args) {
	long offset, segment_size;

	server_thread_params(((_thread_args*)args)->server_sock, &offset, &segment_size);
	char *file_segment;
	long write_size = (segment_size <= MAX_WRITE_SIZE ? segment_size: MAX_WRITE_SIZE);

	file_segment = (char*) malloc(write_size * sizeof(*file_segment));
	long bytes_read;

	int fd_write = open(((_thread_args*)args)->file_path ,O_RDWR);

	lseek(fd_write, offset, SEEK_SET);

	while(segment_size != 0){
		bytes_read = recv(((_thread_args*)args)->server_sock, file_segment, write_size, 0);
		if(bytes_read < 0)
			fprintf(stderr, "\nErro ao tentar ler arquivo pedido\n\n");

		write(fd_write, file_segment, bytes_read);
		segment_size -= bytes_read;
		if(write_size >= segment_size){
			write_size = segment_size;
		}
	}

	free(file_segment);
	free(args);
	close(fd_write);

	return NULL;
}

static void initialize_thread_args(struct thread_args *args, int thread_number, int server_sock, char *file_path) {
	args->thread_number = thread_number;
	args->server_sock = server_sock;
	args->file_path = file_path;
}

