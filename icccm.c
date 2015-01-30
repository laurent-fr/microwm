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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "icccm.h"


// the X11 display must be declared somewhere
extern Display *display;

/// \brief Get a X Window Name
///
/// \param w a X window
/// \param name of the window (output)
/// \return 0 for success
///
///  \note name must be freed with free()
///
Status get_window_name(Window w, char **name) {
    int    status;
    XTextProperty text_prop;
    char **list;
    int    num;

    status = XGetWMName(display, w, &text_prop);
    if (!status || !text_prop.value || !text_prop.nitems) return 0;
    status = XmbTextPropertyToTextList(display, &text_prop, &list, &num);
    if (status < Success || !num || !*list) return 0;
    XFree(text_prop.value);
    *name = (char *)strdup(*list);
    XFreeStringList(list);
    return 1;
}

/// \brief Check if the window has WM_DELETE_WINDOW property
///
/// \param w a X window
/// \return True if has WM_DELETE_WINDOW, False otherwise
///
Bool has_wm_delete_window(Window w) {

    Bool has_property=False;

	// find supported protocols
    Atom *protocols,*protocol;
    int protocols_count=0;
	XGetWMProtocols(display,w,&protocols,&protocols_count);
    int i;
    for (i=0,protocol=protocols;i<protocols_count;i ++,protocol++) {
        //printf("Atom: %s\n",XGetAtomName(display,*protocol));
        char *name = XGetAtomName(display,*protocol);
        int result = strcmp(name,"WM_DELETE_WINDOW");
        XFree(name);
        if (!result) { has_property = True ; break ;}
    }
	XFree(protocols);

    return has_property;
}

/// \brief read WM_NORMAL_HINTS window property
///
/// \param w a X window
/// \param hints (output) a iccm_size_hints structure with the data
///
/// undefined values are set to -1
///
void get_wm_normal_hints(Window w,icccm_size_hints *hints) {

    hints->height_inc=-1;
    hints->width_inc=-1;
    hints->max_height=-1;
    hints->max_width=-1;
    hints->min_height=-1;
    hints->min_width=-1;

    XSizeHints hints_return;
    long supplied_return=0;
    Status status = XGetWMNormalHints(display,w,&hints_return,&supplied_return);
    if (status==0) return;

    if (supplied_return&PMinSize) {
        hints->min_width = hints_return.min_width;
        hints->min_height = hints_return.min_height;
    }

    if (supplied_return&PMaxSize) {
        hints->max_width = hints_return.max_width;
        hints->max_height = hints_return.max_height;
    }

    if (supplied_return&PResizeInc) {
        hints->width_inc = hints_return.width_inc;
        hints->height_inc = hints_return.height_inc;
    }


}
