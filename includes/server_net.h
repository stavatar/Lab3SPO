#ifndef _SERVER_NET_H_
#define _SERVER_NET_H_

#include "net.h"

int init_server_socket(unsigned short port);
int accept_connection(int sock);
int send_server_message(int sock, server_message_t *msg, int flags);
int receive_client_text_message(int sock, client_text_message_t *out);

#endif
