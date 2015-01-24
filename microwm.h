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
    north =1,
    south =2,
    east = 4,
    west = 8
};


// prototypes
// **********

void connect_x_server();

void create_window_decoration(Window window);

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

void reparent_root_windows();

void main_event_loop();


#endif
