#ifndef _UTIL_H_
#define _UTIL_H_

#include "net.h"
#include <errno.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#define MSG_BUFLEN (80 * 25 + 1)

typedef struct list {
  struct list *next;
  struct list *prev;
  void *data;
} list_t;
typedef struct client_shared_data {
    pthread_t display_thread;
    pthread_t receiver_thread;
    pthread_t sender_thread;
    pthread_cond_t display_cond;
    pthread_mutex_t history_mutex;
    pthread_mutex_t cursor_mutex;
    list_t *history_head;
    list_t *history_tail;
    char* name;
    int socket;
    int cursor;
    int down_pressed;
    int is_running; // 1-running, 0-ended
} client_shared_data_t;
list_t *new_list(void *data);
list_t *list_insert_after(list_t *node, void *data);
list_t *list_insert_before(list_t *node, void *data);
void list_remove(list_t *node);
void create_message(char *buf, size_t buflen, server_message_t *msg,char *name_client);
void show_message(WINDOW *window, server_message_t *msg);

#endif
