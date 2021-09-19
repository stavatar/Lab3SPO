#include "../includes/client_net.h"
#include "../includes/util.h"
#include <arpa/inet.h>
#include <endian.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int init_client_socket(char const *address, unsigned short port) {
    int sock;
    // создаем сокет, указывая
    // семейство протоколов создаваемого сокета
    // тип SOCK_STREAM (надёжная потокоориентированная служба (сервис) или потоковый сокет)
    // протокол IPPROTO_TCP
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Create client socket error\n");
    }

    struct sockaddr_in addr = {0};
    // Указываем тип сокета Интернет
    addr.sin_family = PF_INET;
    // Указываем порт сокета
    // htons преобразует узловой порядок расположения байтов положительного короткого целого hostshort в сетевой порядок расположения байтов.
    addr.sin_port = htons(port);
    // проверка адреса на корректность путем преобразования строку символов address в сетевой адрес (типа af),
    // затем копирование полученной структуры с адресом в &addr.sin_addr.
    switch (inet_pton(PF_INET, address, &addr.sin_addr)) {
        case 0:
            perror("Incorrect address");
            return -1;
        case -1:
            return -1;
        case 1:
            break;
    }
    // Устанавливаем соединение
    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("Error connect()");
        close(sock);
        return -1;
    }else{fprintf(stderr, "CONNECTED\n");}

    return sock;
}

int send_client_text_message(int sock, client_text_message_t *msg) {
    // сначала отправляем имя получателя с флагом, что имеются еще сообщения
    if (send_msg(sock, msg->receiver_name_len, msg->receiver_name, MSG_MORE) < 0) {
        fprintf(stderr, "Send receiver_name error\n");
    }
    if (send_msg(sock, msg->text_len, msg->text, 0) < 0) {
        fprintf(stderr, "Send msg error\n");
    }
    return 1;
}

static int receive_server_text_message(int sock, server_text_message_t *out) {
    if (receive_msg(sock, &out->sender_name_len, &out->sender_name) < 0) {
        fprintf(stderr, "Receive sender_name error\n");
    }
    if (receive_msg(sock, &out->receiver_name_len, &out->receiver_name) < 0) {
        fprintf(stderr, "Receive receiver_name error\n");
    }
    if (receive_msg(sock, &out->text_len, &out->text) < 0) {
        fprintf(stderr, "Receive msg error\n");
    }
    return 1;
}

static int
receive_server_member_notification(int sock,
                                   server_member_notification_t *out) {
    if (receive_msg(sock, &out->length, &out->name) < 0) {
        fprintf(stderr, "Receive msg error\n");
    }
    return 1;
}

int receive_server_message(int sock, server_message_t *out) {
    int64_t timestamp;
    if (recv(sock, &timestamp, sizeof(int64_t), MSG_WAITALL) < 0) {
        fprintf(stderr, "Receive time error\n");
    }
    if (recv(sock, &out->id, sizeof(uint32_t), MSG_WAITALL) < 0) {
        fprintf(stderr, "Receive id error\n");
    }
    out->id = ntohl(out->id);
    out->timestamp = be64toh(timestamp);

    uint8_t type;
    if (recv(sock, &type, sizeof(uint8_t), MSG_WAITALL) < 0) {
        fprintf(stderr, "Receive type error\n");
    }
    out->type = type;

    switch (type)
    {
        case TEXT_MESSAGE:
            if (receive_server_text_message(sock, &out->text) < 0) {
                fprintf(stderr, "Receive server msg error\n");
            }
            printf("%s",&out->text.text);
            break;
        case JOIN_NOTIFICATION:
        case LEAVE_NOTIFICATION:
            if (receive_server_member_notification(sock, &out->member) < 0) {
                fprintf(stderr, "Receive server member notification error\n");
            }
            printf("%s",&out->member.name);
            break;
        default:
            fprintf(stderr, "Unknown type: %" PRIu8 "\n", type);
            return -1;
    }
    return 1;
}
