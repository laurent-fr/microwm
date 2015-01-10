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

/*
struct wm_window {

	RB_ENTRY(widget) link;
	

}
*/

typedef struct Widget_s {

//	RB_ENTRY(widget) link;
	Window w;
	widget_type type;
	char *text;
	char **bitmap;
} Widget ;



void connect_x_server();

Widget *create_widget(widget_type type,Window parent,int x,int y,unsigned int w,unsigned int h,XColor color);

void draw_widget_button(Widget *wg);
void draw_widget_title_bar(Widget *wg);
void draw_widget_decoration(Widget *wg);

void create_window_decoration(Window window);
void main_event_loop();


#endif
