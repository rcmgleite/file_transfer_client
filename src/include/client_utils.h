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

char *build_file_path(char* file_name, char *dir_path);
int create_connection(char *host, char *port);
void parse_header(int con_sock, int *num_threads, int *file_size);
void server_thread_params(int con_sock, int *offset, int *segment_size);
void read_line(int fd, char conent[]);

#endif /* CLIENT_UTILS_H_ */
