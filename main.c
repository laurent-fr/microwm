#include "microwm.h"


int main(int ARGC, char *ARGV[]) {

    // connect to X server
    connect_x_server();

    // reparent existing windows
    reparent_root_windows();

    // main event loops
    main_event_loop();


    return 0;
}
