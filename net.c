#include "includes/net.h"
#include "includes/util.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>


int receive_msg(int sock, uint32_t *len_out, char **str_out) {
    if (recv(sock, len_out, sizeof(uint32_t), MSG_WAITALL) < 0) {
        fprintf(stderr, "Receive msg length error\n");
    }
    *len_out = ntohl(*len_out);
    *str_out = malloc(*len_out + 1);
    if (!*str_out) {
        perror("Out of memory");
        return -1;
    }
    if (*len_out) {
        if (recv(sock, *str_out, *len_out, MSG_WAITALL) < 0) {
            fprintf(stderr, "Receive msg error\n");
        }
    }
    str_out[0][*len_out] = 0;
    return 1;
}

int close_connection(int sock) {
    int shutdown_retcode = shutdown(sock, SHUT_RDWR);
    if (close(sock) < 0) {
        fprintf(stderr, "Socket closing error\n");
    }
    if (shutdown_retcode < 0) {
        fprintf(stderr, "Socket shutdown error\n");
    }
    return 0;
}

int send_msg(int socket, uint32_t len, char *str, int flags) {
    uint32_t len_to_send = htonl(len);
    // отправка сначала длины пересылаемого сообщения
    if (send(socket, &len_to_send, sizeof(uint32_t), flags | MSG_MORE) < 0) {
        fprintf(stderr, "Send msg length error\n");
    }
    // отправка самого сообщения
    if (send(socket, str, len, flags) < 0) {
        fprintf(stderr, "Send msg error\n");
    }
    return 1;
}
