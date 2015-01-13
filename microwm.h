#ifndef microwm_h
#define microwm_h

#include <X11/Xlib.h> // Every Xlib program must include this
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h> // freetype
#include <X11/xpm.h>


#define NIL (0)       // A name for the void pointer


#define deco_l 4
#define deco_r 4
#define deco_t 20
#define deco_b 4

typedef enum {
    wg_frame,
    wg_decoration,
    wg_title_bar,
    wg_button
}  widget_type ;

enum  {
    bm_close,
    bm_full,
    bm_iconify
} ;

/*
struct wm_window {

}
*/

typedef struct {
    unsigned int width;
    unsigned int height;
    int top;
    int left;
    int right;
    int bottom;
} WgGeometry ;


typedef struct {
    Window w;
    Window parent;
    widget_type type;
    WgGeometry geom;
    int bmp;
    char *text;
} Widget ;


void connect_x_server();

Widget *create_widget(widget_type type,Window parent,WgGeometry *geometry,XColor color);

void draw_widget_button(Widget *wg);
void draw_widget_title_bar(Widget *wg);
void draw_widget_decoration(Widget *wg);

void create_window_decoration(Window window);
void main_event_loop();


#endif
