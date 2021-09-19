#include "includes/display_lib.h"
 void prepare_message(rendered_message_t *msg, int window_width,char* name_client) {
    create_message(msg->buffer, MSG_BUFLEN, &msg->msg,name_client);
    int x = 0;
    msg->height = 0;
    for (char *c = msg->buffer; *c; ++c) {
        if (0x20 <= *c && *c <= 0x7f) {
            if (++x == window_width)
                msg->height++;
        } else if ('\n' == *c) {
            x = 0;
            msg->height++;
        }
    }
}

  int min(int a, int b) { return a < b ? a : b; }

  int max(int a, int b) { return a > b ? a : b; }

  int find_max_cursor_pos(list_t *head, int height) {
     int max_pos = 0;
     list_t *iter;
     for (iter = head; iter; iter = iter->next)
         max_pos += ((rendered_message_t *) iter->data)->height;
     return max(0, max_pos - height);
 }

  int find_real_cursor_pos(int cursor, int max_pos) {
     return max(0, cursor < 0 ? max_pos : min(cursor, max_pos));
 }
 int display_part(WINDOW *window, rendered_message_t *msg,int sock)
  {
      int maxy = getmaxy(window);
      char *c;
          //if(msg->msg.type==SERVER_CLOSE)
           //   close_connection(sock);
      for (c = msg->buffer; *c; ++c)
      {
          if (getcury(window) < maxy - 1) {
              waddch(window, *c);
             // wrefresh(window);
          }
      }

      return getcury(window) < maxy - 1;
  }

 void output_msg(client_shared_data_t *shared,WINDOW* window,int cursor,int max_pos)
 {
     list_t *iter;
     iter = shared->history_head;
     int shift = 0;
     while (iter && shift < cursor)
     {
         rendered_message_t *current=iter->data;
         shift += ((rendered_message_t *) iter->data)->height;
         iter = iter->next;
     }

     wclear(window);
     wmove(window, 0, 0);
     if (iter) {
         rendered_message_t *current=iter->data;
         display_part(window, iter->data,shared->socket);
         wscrl(window, shift - cursor);
         iter = iter->next;
     }

     while (iter && display_part(window, iter->data,shared->socket))
     {
         rendered_message_t *current=iter->data;
         iter = iter->next;
     }

     if (max_pos)
         wprintw(window, "%d lines above, %d below", shift, max_pos - shift);
     wrefresh(window);
 }

