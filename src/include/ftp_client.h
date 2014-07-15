/*
 * ftp_client.h
 *
 *  Created on: Jun 30, 2014
 *      Author: rafael
 */

#ifndef FTP_CLIENT_H_
#define FTP_CLIENT_H_
#include <pthread.h>		//lib das threads

#define SERVER_PORT	30000

#define TRUE 1
#define FALSE 0

typedef struct thread_args{
	int server_sock;
	int thread_number;
	char *file_path;
}_thread_args;

void *thread_function(void *args);
void initialize_thread(pthread_t *thread, struct thread_args *args, int thread_number, int server_sock, char *file_path);
void clean_up(pthread_t *threads, int *number_of_threads,
		long *file_size, long *curr_offset, char *file_path);
#endif /* FTP_CLIENT_H_ */
