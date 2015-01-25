// microWM - a microscopic Window Manager
// Copyright (C) 2015 Laurent FRANCOISE - gihtub_at_diygallery_dot_com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include <signal.h>

#include "microwm.h"

/// \brief Called on exit
///

void int_handler() {
    printf("CTRL+C\n");
    disconnect_x_server();
    exit(0);
}

/// \brief The main program
///
int main(int ARGC, char *ARGV[]) {

    // register signals
    signal(SIGINT, int_handler);

    // connect to X server
    connect_x_server();

    // reparent existing windows
    reparent_root_windows();

    // main event loops
    main_event_loop();

    return 0;
}
