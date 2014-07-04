/*
 * ftp_client.h
 *
 *  Created on: Jun 30, 2014
 *      Author: rafael
 */

#ifndef FTP_CLIENT_H_
#define FTP_CLIENT_H_
#include <pthread.h>		//lib das threads

#define FIRST_GUESS_OFFSET 32768

typedef struct thread_args{
	int server_sock;
	int thread_number;
	int fd_to_write;
}_thread_args;

void *thread_function(void *args);
void initialize_thread(pthread_t *thread, struct thread_args *args, int thread_number, int server_sock, int fd_to_write);
void clean_up(int fd_to_write, pthread_t *threads, int *number_of_threads,
		int *file_size, int *curr_offset);
#endif /* FTP_CLIENT_H_ */
