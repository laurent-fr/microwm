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

#ifndef microwm_h
#define microwm_h

#include <X11/Xlib.h> // Every Xlib program must include this
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h> // freetype
#include <X11/xpm.h>

#include "widgets.h"

enum {
    north =1, ///< top side of a window
    south =2, ///< bottom side of a window
    east = 4, ///< left side of a window
    west = 8  ///< right side of a window
};


// prototypes
// **********

void connect_x_server();
void disconnect_x_server();

void create_window_decoration(Window window);

void paint_full_button(Widget *button,XExposeEvent e);
void paint_title_bar(Widget *title_bar,XExposeEvent e);

void on_click_title_bar(Widget *title_bar,XButtonPressedEvent e);
void on_click_decoration(Widget *decoration,XButtonPressedEvent e);
void on_click_close(Widget *button,XButtonPressedEvent e);
void on_click_full(Widget *button,XButtonPressedEvent e);
void on_click_iconify(Widget *button,XButtonPressedEvent e);

void on_motion_decoration(Widget *decoration,XMotionEvent e);

void on_unmap_xclient(Widget *xclient,XUnmapEvent e);

void on_expose_event(XExposeEvent e);
void on_buttonpress_event(XButtonPressedEvent e);
void on_motion_event(XMotionEvent e);
void on_unmap_event(XUnmapEvent e);

void on_configure_request(XConfigureRequestEvent e);

void reparent_root_windows();

void main_event_loop();


#endif
