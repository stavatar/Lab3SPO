#include "includes/client.h"
#include "includes/server.h"
#include <ncurses.h>
#include <signal.h>
void my_handler()
{


}
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("%s","Invalid number of arguments\n");
        return -1;
    }

    initscr();
    cbreak();
    noecho();
    int curs_state = curs_set(0);
    int return_code;

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    if (argv[1][0] == 'c')
    {
        printf("%s","Client run\n");
        return_code =  run_client(argc, argv);
    } else {
        printf("%s","Server run\n");

        return_code = run_server(argc, argv);
    }
    curs_set(curs_state);
    endwin();
    return return_code;
}
