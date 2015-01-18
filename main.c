#include <signal.h>

#include "microwm.h"

void int_handler() {
    printf("CTRL+C\n");
    //wg_destroy_all();
    exit(0);
}

int main(int ARGC, char *ARGV[]) {

    // connect to X server
    connect_x_server();
    // register ctrl+c
    //signal(SIGINT, int_handler);

    // connect to X server
    connect_x_server();

    // reparent existing windows
    reparent_root_windows();

    // main event loops
    main_event_loop();


    return 0;
}
