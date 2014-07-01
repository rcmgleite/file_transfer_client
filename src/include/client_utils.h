/*
 * client_utils.h
 *
 *  Created on: Jun 30, 2014
 *      Author: rafael
 */

#ifndef CLIENT_UTILS_H_
#define CLIENT_UTILS_H_

int create_connection(char *host, char *port);
void parse_header(int con_sock, int *num_threads, int *file_size);

#endif /* CLIENT_UTILS_H_ */
