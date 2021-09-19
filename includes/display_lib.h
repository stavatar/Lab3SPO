#include "../includes/util.h"
#ifndef DISPLAY_LIB_H
#define DISPLAY_LIB_H

typedef struct rendered_message {
    server_message_t msg;
    int height;
    char buffer[MSG_BUFLEN];
} rendered_message_t;

  void prepare_message(rendered_message_t *msg, int window_width,char* name_client);
  int display_part(WINDOW *window, rendered_message_t *msg,int sock);
  void output_msg(client_shared_data_t *shared,WINDOW* window,int cursor,int max_pos);
  int min(int a, int b);
  int max(int a, int b);
  int find_max_cursor_pos(list_t *head, int height);
  int find_real_cursor_pos(int cursor, int max_pos);
#endif // DISPLAY_LIB_H
