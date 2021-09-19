#ifndef _CLIENT_NET_H_
#define _CLIENT_NET_H_

#include "net.h"

int init_client_socket(char const *address, unsigned short port);
int send_client_text_message(int sock, client_text_message_t *msg);
int receive_server_message(int sock, server_message_t *out);

#endif
