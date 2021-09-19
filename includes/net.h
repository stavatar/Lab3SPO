#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include <inttypes.h>
#include <time.h>

typedef enum server_message_type {
  TEXT_MESSAGE,
  JOIN_NOTIFICATION,
  LEAVE_NOTIFICATION,
  SERVER_CLOSE
} server_message_type_t;

typedef struct client_text_message {
  uint32_t receiver_name_len;
  uint32_t text_len;
  char *receiver_name;
  char *text;
} client_text_message_t;

typedef struct server_text_message {
  uint32_t sender_name_len;
  uint32_t receiver_name_len;
  uint32_t text_len;
  char *sender_name;
  char *receiver_name;
  char *text;
} server_text_message_t;

typedef struct server_member_notification {
  uint32_t length;
  char *name;
} server_member_notification_t;

typedef struct server_message {
  time_t timestamp;
  uint32_t id;
  server_message_type_t type;
  union {
    server_member_notification_t member;
    server_text_message_t text;
  };
} server_message_t;

int close_connection(int sock);
int send_msg(int socket, uint32_t len, char *str, int flags);
int receive_msg(int sock, uint32_t *len_out, char **str_out);

#endif
