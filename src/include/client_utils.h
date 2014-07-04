/*
 * client_utils.h
 *
 *  Created on: Jun 30, 2014
 *      Author: rafael
 */

#ifndef CLIENT_UTILS_H_
#define CLIENT_UTILS_H_

#define ROOT_PATH "/home/rafael/Desktop/rafael/C/proj_redes_client/Debug/"

char *build_file_path(char* file_name);
int create_connection(char *host, char *port);
void parse_header(int con_sock, int *num_threads, int *file_size);
void server_thread_params(int con_sock, int *offset, int *segment_size);
void read_line(int fd, char conent[]);

#endif /* CLIENT_UTILS_H_ */
