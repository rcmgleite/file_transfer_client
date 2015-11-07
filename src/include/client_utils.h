/*
 * client_utils.h
 *
 *  Created on: Jun 30, 2014
 *      Author: rafael
 */

#ifndef CLIENT_UTILS_H_
#define CLIENT_UTILS_H_

#define MAX_ITER 1000000
#define PROGRAM_NAME "proj_redes_client"

char* format_file_path(char* file_name);
int create_connection(char *host, char *port);
void parse_header(int con_sock, int *num_threads, long *file_size);
void server_thread_params(int con_sock, long *offset, long *segment_size);
void read_line(int fd, char conent[]);
void wait_init(int con_sock);

#endif /* CLIENT_UTILS_H_ */
