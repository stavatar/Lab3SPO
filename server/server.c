#include "../includes/server.h"
#include "../includes/net.h"
#include "../includes/server_net.h"
#include "../includes/util.h"
#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>

typedef struct server_data {
    pthread_t initcon_thread;
    pthread_t queue_thread;
    pthread_cond_t queue_cond;
    pthread_mutex_t history_mutex;
    pthread_mutex_t queue_mutex;
    pthread_mutex_t connection_mutex;
    list_t *connections;
    list_t *message_history;
    list_t *message_queue;
    int sock;
    int is_running;
} server_data_t;

typedef struct server_connection_data {
    int conn;//номер сокета
    char *client_name;
    uint32_t client_name_len;
    pthread_t thread;
    server_data_t *shared;
    list_t *list_pos;
    int is_active;
} server_connection_data_t;

void send_mes_to_clients(server_data_t *shared,server_message_t *msg)
{
    list_t *iter;
    for (iter = shared->connections; iter; iter = iter->prev)
    {
        server_connection_data_t *cd = iter->data;
        if (cd->is_active)
        {
            if (msg->type != TEXT_MESSAGE || !msg->text.receiver_name_len ||
                !strcmp(msg->text.receiver_name, cd->client_name) ||
                !strcmp(msg->text.sender_name, cd->client_name))
                if (send_server_message(cd->conn, msg, MSG_DONTWAIT) <  0)
                {
                    fprintf(stderr, "Send server msg error\n");
                }
        }
    }
}
static int queue_routine(server_data_t *shared);
static void *queue_routine_voidptr(void *ptr) {
    queue_routine(ptr);
    return NULL;
}

int queue_routine (server_data_t *shared) {
    uint32_t id = 0;
    scrollok(stdscr, TRUE);

    pthread_mutex_lock(&shared->queue_mutex);
    while (shared->is_running)
    {

        pthread_cond_wait(&shared->queue_cond, &shared->queue_mutex);
        while (shared->message_queue)
        {

            if (!shared->message_queue)
                shared->message_queue = shared->message_history;
            list_t *iter;

            iter = shared->message_queue;
            while (iter )
            {
                server_message_t *current=iter->data;
                iter = iter->next;
            }
            list_t *iter1;

            server_message_t *msg = shared->message_queue->data;

            msg->timestamp = time(NULL);
            msg->id = ++id;

            pthread_mutex_lock(&shared->history_mutex);
            pthread_mutex_unlock(&shared->history_mutex);

            show_message(stdscr, msg);
            refresh();

            send_mes_to_clients(shared,msg);

            shared->message_queue = shared->message_queue->next;
            if (shared->message_queue)
                list_remove(shared->message_queue->prev);
        }
    }
    pthread_mutex_unlock(&shared->queue_mutex);
    return 0;
}

static void queue_append(server_data_t *shared, server_message_t *msg)
{
    pthread_mutex_lock(&shared->queue_mutex);
   shared->message_history= list_insert_after(shared->message_history, msg);
   if (!shared->message_queue)
       shared->message_queue = shared->message_history;
    pthread_mutex_unlock(&shared->queue_mutex);
    pthread_cond_signal(&shared->queue_cond);
}

static int connection_receive_text(server_connection_data_t *self);
static void *connection_receive_text_voidptr(void *ptr) {
    connection_receive_text(ptr);
    return NULL;
}

int connection_receive_text(server_connection_data_t *self) {
    int retcode = 1;
    while (retcode > 0 && self->is_active &&self->shared->is_running)
    {
        client_text_message_t cmsg;
        retcode = receive_client_text_message(self->conn, &cmsg);
        server_message_t *msg = malloc(sizeof(server_message_t));

        switch (retcode) {
            case 0:
                msg->type = LEAVE_NOTIFICATION;
                msg->member.name = self->client_name;
                msg->member.length = self->client_name_len;
                queue_append(self->shared, msg);
                break;
            case -1:
                free(msg);
                break;
            case 1:
                msg->type = TEXT_MESSAGE;
                msg->text.sender_name = self->client_name;
                msg->text.sender_name_len = self->client_name_len;
                msg->text.receiver_name = cmsg.receiver_name;
                msg->text.receiver_name_len = cmsg.receiver_name_len;
                msg->text.text = cmsg.text;
                msg->text.text_len = cmsg.text_len;
                queue_append(self->shared, msg);
                break;
        }
    }

    self->is_active = 0;
    return retcode;
}

static int init_connection(server_data_t *shared);
static void *init_connection_voidptr(void *ptr) {
    init_connection(ptr);
    return NULL;
}

int init_connection(server_data_t *shared) {
    while (shared->is_running) {
        int conn = accept_connection(shared->sock);
        // принять запрос на установку соединения
        if (conn <  0) {
            fprintf(stderr, "Create accept_connection thread error\n");
        }
        server_connection_data_t *data = malloc(sizeof(server_connection_data_t));
        // получение имени клиента
        int received_name =
                receive_msg(conn, &data->client_name_len, &data->client_name);
        if (received_name == 1) {
            // блокируем мьютекс. Если мьютекс уже заблокирован, то ждем, пока мьютекс не станет доступным.
            pthread_mutex_lock(&shared->history_mutex);

            data->shared = shared;
            data->conn = conn;//номер сокета
            data->is_active = 1;

            if (pthread_create(&data->thread, NULL,
                               connection_receive_text_voidptr, data) !=  0) {
                fprintf(stderr, "Create connection_routine thread error\n");
            }

            pthread_mutex_lock(&shared->connection_mutex);
            shared->connections = data->list_pos =
                    list_insert_after(shared->connections, data);
            pthread_mutex_unlock(&shared->connection_mutex);

            list_t *iter;
            for (iter = shared->message_history; iter; iter = iter->prev)
                send_server_message(conn, iter->data, MSG_DONTWAIT);
            pthread_mutex_unlock(&shared->history_mutex);

            server_message_t *msg = malloc(sizeof(server_message_t));
            msg->member.length = data->client_name_len;
            msg->member.name = data->client_name;
            msg->type = JOIN_NOTIFICATION;
            queue_append(shared, msg);
        } else {
            free(data);
        }
    }
    return 0;
}

int run_server(int argc, char **argv) {
    scrollok(stdscr, TRUE);
    if (argc < 3) {
        printf("%s","Invalid number of arguments for server mode\n");
        return -1;
    }
    server_data_t shared;
    shared.sock = init_server_socket(atoi(argv[2]));
    shared.connections = NULL;
    shared.message_queue = NULL;
    shared.message_history = NULL;

    if (shared.sock < 0) {
        fprintf(stderr, "Init server socket error\n");
    }
    if (pthread_cond_init(&shared.queue_cond, NULL) != 0) {
        fprintf(stderr, "Init queue_cond error\n");
    }
    if (pthread_mutex_init(&shared.connection_mutex, NULL) !=  0) {
        fprintf(stderr, "Init connection_mutex error\n");
    }
    if (pthread_mutex_init(&shared.history_mutex, NULL) !=  0) {
        fprintf(stderr, "Init history_mutex error\n");
    }
    if (pthread_mutex_init(&shared.queue_mutex, NULL) !=  0) {
        fprintf(stderr, "Init queue_mutex error\n");
    }

    shared.is_running = 1;
    if (pthread_create(&shared.initcon_thread, NULL,
                       init_connection_voidptr, &shared) !=  0) {
        fprintf(stderr, "Create acceptor_thread error\n");
    }
    if (pthread_create(&shared.queue_thread, NULL,
                       queue_routine_voidptr, &shared) !=  0) {
        fprintf(stderr, "Create acceptor_thread error\n");
    }

    int key;
    do {
        key = getch();
    } while (key != 'q' && key != 'Q');

    shared.is_running = 0;
    pthread_cond_signal(&shared.queue_cond);
    pthread_join(shared.queue_thread, NULL);


    while (shared.connections) {
        list_t *prev = shared.connections->prev;
        server_connection_data_t *data = shared.connections->data;
        data->is_active = 0;

        close_connection(data->conn);
        pthread_join(data->thread, NULL);
        free(data->client_name);
        free(data);
        list_remove(shared.connections);
        shared.connections = prev;
    }

    close_connection(shared.sock);
    pthread_join(shared.initcon_thread, NULL);
    pthread_mutex_destroy(&shared.history_mutex);
    pthread_mutex_destroy(&shared.queue_mutex);
    pthread_mutex_destroy(&shared.connection_mutex);

    return 0;
}
