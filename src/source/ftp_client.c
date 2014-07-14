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
#include <time.h>

#include "ftp_client.h"
#include "client_utils.h"

pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
/**
 *	O programa é inicializado com 2 parâmetros: ip e porta (igual ao telnet)
 **/
int main(int argc, char *argv[]){
	if(argc != 3){
		fprintf(stderr, "Passe os parametros 'ip' e 'porta' para que o cliente possa se conectar ao cliente.\n");
		exit(1);
	}

	/*
	 *	encontra o diretório de execução do programa
	 **/
	char exec_path[255];
	readlink("/proc/self/exe", exec_path, 255);

	struct rlimit rlp;
	getrlimit(RLIMIT_NOFILE, &rlp);
	rlp.rlim_cur = 4000;
	setrlimit(RLIMIT_NOFILE, &rlp);

	/**
	 *	Variáveis necessárias
	 **/
	int curr_offset = 0;

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

	/**
	 * 	Abro o arquivo que vou escever
	 **/
	char *path_to_write = build_file_path(to_say, exec_path);
	fprintf(stderr, "path to received file: %s\n", path_to_write);
	int fd_to_write = open(path_to_write ,O_RDWR | O_CREAT, S_IRUSR|S_IWUSR);
	if(fd_to_write == 1){
		fprintf(stderr, "Error while trying to create the new file\n Shutting down");
		close(con_sock);
		exit(1);
	}

	int num_threads, file_size;

	/**
	 * 	Nos headers eu tenho o número de threads e o tamanho do arquivo que vai ser recebido
	 **/
	parse_header(con_sock, &num_threads, &file_size);

	fprintf(stderr, "num_threads: %d\n", num_threads);

	/**
	 *	Daqui pra frente lê o que o servidor manda e monta o arquivo final
	 *
	 *	Alocação de memória para threads e para a estrutura de argumentos
	 **/
	pthread_t *threads;
	threads = (pthread_t *)malloc(num_threads * sizeof(*threads));

	/*
	 *	Contagem de tempo da transferência
	 **/
	clock_t begin, end;
	double time_spent;

	begin = clock();

	/**
	 *	Inicialização das threads
	 **/
	int i;
	for(i = 0; i < num_threads; i++){
		struct thread_args *args;
		args = (struct thread_args *)malloc(sizeof(*args));
		initialize_thread(&threads[i], args, i, con_sock, path_to_write);
	}

	//Apenas para o programa esperar as threads executarem
	for (i = 0; i < num_threads; i++){
		pthread_join(threads[i], NULL);
	}


    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    fprintf(stderr, "Aqruivo recebido!\nTime elapsed: %f s\n", time_spent);

	// Limpo todos os dados para a próxima requesição
	clean_up(fd_to_write, threads, &num_threads, &curr_offset, &file_size, path_to_write);

	close(con_sock);

	return 0;
}

void *thread_function(void *args){
	/**
	 * 	Criação das conexões TCP entre threads.. função espelho da do server
	 **/
	/*TODO - IP NÃO HARD-CODED e 30000 não hard-coded*/
	char new_port[100];
	sprintf(new_port, "%d", SERVER_PORT + 1 + ((_thread_args*)args)->thread_number);
	int trans_sock = create_connection("127.0.0.1", new_port);

	/**
	 *	Lê o offset e o segment_size que o servidor enviar para remontar o arquivo
	 **/
	int offset, segment_size;

	server_thread_params(trans_sock, &offset, &segment_size);

	char *file_segment;
	file_segment = (char*) malloc(segment_size * sizeof(*file_segment));
	int bytes_read;

	int fd_write = open(((_thread_args*)args)->file_path ,O_RDWR | O_CREAT, S_IRUSR|S_IWUSR);

	/**
	 *	lseek vai mudar o ponteiro do arquivo para escrever no local correto
	 **/
	lseek(fd_write, offset, SEEK_SET);

	while(segment_size != 0){
		bytes_read = recv(trans_sock, file_segment, segment_size, 0);
		if(bytes_read < 0)
			fprintf(stderr, "\nErro ao tentar ler arquivo pedido\n\n");

		write(fd_write, file_segment, bytes_read);
		segment_size -= bytes_read;
	}

	/**
	 *	Libero a memória do file_segment
	 **/
	free(file_segment);
	free(args);

	/*Fecha a conexão TCP*/
	close(trans_sock);

	return NULL;
}

void initialize_thread(pthread_t *thread, struct thread_args *args, int thread_number, int server_sock, char *file_path){
	args->thread_number = thread_number;
	args->server_sock = server_sock;
	args->file_path = file_path;
	pthread_create(thread, NULL, thread_function, (void*)args);
}

void clean_up(int fd_to_write, pthread_t *threads, int *number_of_threads,
		int *file_size, int *curr_offset, char *file_path){
    close(fd_to_write);
    free(threads);
    free(file_path);
	*number_of_threads = 0;
	*file_size = 0;
	*curr_offset = 0;
}
