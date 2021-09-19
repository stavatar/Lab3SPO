#include "includes/util.h"
#include <ncurses.h>
#include <stdlib.h>
#define MSG_BUFLEN (80 * 20 + 1)

void show_message(WINDOW *window, server_message_t *msg) {
    static char buf[MSG_BUFLEN];
    create_message(buf, sizeof(buf), msg,NULL);
    waddnstr(window, buf, sizeof(buf));
}

void create_message(char *buf, size_t buflen, server_message_t *msg,char *name_client) {
    char *pointer = buf;
    size_t shift;
    struct tm *time = localtime(&msg->timestamp);
    if(name_client)
    {
        if (msg->text.receiver_name_len)
          {
                if(!(strcmp(name_client,msg->text.receiver_name)==0)&&!(strcmp(name_client,msg->text.sender_name)==0))
                {
                   return;
                }

           }

    }
    shift = snprintf(pointer, buflen, "%02d:%02d:%02d | ", time->tm_hour,
                     time->tm_min, time->tm_sec);
    pointer += shift;
    buflen -= shift;

    switch (msg->type) {
        case JOIN_NOTIFICATION:
            shift = snprintf(pointer, buflen, "User %s joined\n", msg->member.name);
            break;
        case LEAVE_NOTIFICATION:
            shift = snprintf(pointer, buflen, "User %s left chat\n", msg->member.name);
            break;
        case TEXT_MESSAGE:
         {
               shift = snprintf(pointer, buflen, "Sender: %s|", msg->text.sender_name);

                pointer += shift;
                buflen -= shift;
                if (msg->text.receiver_name_len) {
                    shift = snprintf(pointer, buflen, " -> |Receiver: %s|", msg->text.receiver_name);
                    pointer += shift;
                    buflen -= shift;
                }
                shift = snprintf(pointer, buflen, ": %s\n", msg->text.text);
          }
            break;
    }
}


list_t *new_list(void *data) {
    list_t *node = malloc(sizeof(list_t));
    if (!node) {
        perror("Out of memory");
        return NULL;
    }
    node->next = NULL;
    node->prev = NULL;
    node->data = data;
    return node;
}

list_t *list_insert_after(list_t *node, void *data) {
    list_t *new_list_t = new_list(data);
    if (!new_list_t)
        return NULL;
    new_list_t->prev = node;
    if (node) {
        new_list_t->next = node->next;
        node->next = new_list_t;
        if (new_list_t->next)
            new_list_t->next->prev = new_list_t;
    }
    return new_list_t;
}

list_t *list_insert_before(list_t *node, void *data) {
    list_t *new_list_t = new_list(data);
    if (!new_list_t)
        return NULL;
    new_list_t->next = node;
    if (node) {
        new_list_t->prev = node->prev;
        node->prev = new_list_t;
        if (new_list_t->prev)
            new_list_t->prev->next = new_list_t;
    }
    return new_list_t;
}

void list_remove(list_t *node) {
    if (node->next)
        node->next->prev = node->prev;
    if (node->prev)
        node->prev->next = node->next;
    free(node);
}
