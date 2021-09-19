#include "../includes/server_net.h"
#include "../includes/util.h"
#include <endian.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int receive_client_text_message(int sock, client_text_message_t *out) {
    if (receive_msg(sock, &out->receiver_name_len, &out->receiver_name) < 0) {
        fprintf(stderr, "Receive receiver name error\n");
    }
    if (receive_msg(sock, &out->text_len, &out->text) < 0) {
        fprintf(stderr, "Receive text length error\n");
    }
    char *buf="LEAVE_NOTIFICATION";

    if(!strcmp(out->text,buf))
        return 0;
    return 1;
}

int init_server_socket(unsigned short port) {
    struct sockaddr_in addr = {0};
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_family = PF_INET;

    int sock;
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Create server socket error\n");
    }

    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("Bind server socket error");
        if (close(sock) < 0) {
            fprintf(stderr, "Server socket closing error\n");
        }
        return -1;
    }

    if (listen(sock, SOMAXCONN) == -1) {
        perror("Listen server socket error");
        if (close(sock) < 0) {
            fprintf(stderr, "Server socket closing error\n");
        }
        return -1;
    }
    return sock;
}

// принять запрос на установку соединения
int accept_connection(int sock) {
    int connection;
    connection = accept(sock, NULL, NULL);
    if (connection  < 0) {
        fprintf(stderr, "Server socket connecting error\n");
    }
    return connection;
}

int send_server_text_message(int sock, server_text_message_t *msg, int flags)
{
    if (send_msg(sock, msg->sender_name_len, msg->sender_name,
                 flags | MSG_MORE) < 0) {
        fprintf(stderr, "Send sender name error\n");
    }
    if (send_msg(sock, msg->receiver_name_len, msg->receiver_name,
                 flags | MSG_MORE) < 0) {
        fprintf(stderr, "Send receiver name error\n");
    }
    if (send_msg(sock, msg->text_len, msg->text, flags) < 0) {
        fprintf(stderr, "Send text msg error\n");
    }
    return 1;
}

int send_server_message(int sock, server_message_t *msg, int flags) {
    int64_t timestamp = htobe64(msg->timestamp);
    uint32_t id = htonl(msg->id);
    uint8_t type = msg->type;
    //printf("timestamp= %ld, id=%u, type=%u \n", timestamp,id,type);

    if (send(sock, &timestamp, sizeof(int64_t), MSG_MORE) < 0) {
        fprintf(stderr, "Send time error\n");
    }
    if (send(sock, &id, sizeof(uint32_t), MSG_MORE) < 0) {
        fprintf(stderr, "Send msg id error\n");
    }
    if (send(sock, &type, sizeof(uint8_t), MSG_MORE) < 0) {
        fprintf(stderr, "Send msg type error\n");
    }
    switch (msg->type) {
        case TEXT_MESSAGE:
            if (send_server_text_message(sock, &msg->text, flags) < 0) {
                fprintf(stderr, "Send server text msg error\n");
            }
            break;
        case JOIN_NOTIFICATION:
        case LEAVE_NOTIFICATION:
            //printf("%s", "send_server_message:msg->type= LEAVE_NOTIFICATION or JOIN_NOTIFICATION\n");
            if (send_msg(sock, msg->member.length, msg->member.name, flags) < 0) {
                fprintf(stderr, "Send server text msg error\n");
            }
            break;
    }
    return 1;
}
