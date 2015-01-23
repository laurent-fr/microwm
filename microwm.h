#ifndef microwm_h
#define microwm_h

#include <X11/Xlib.h> // Every Xlib program must include this
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h> // freetype
#include <X11/xpm.h>


#define NIL (0)       // A name for the void pointer

#define DECORATION_MARGIN 4
#define DECORATION_MARGIN_TOP 20

typedef enum {
    wg_x11,
    wg_decoration,
    wg_title_bar,
    wg_button
}  widget_type ;

enum  {
    bm_close,
    bm_full,
    bm_iconify
} ;

typedef enum {
    wm_normal,
    wm_maximized,
    wm_icon
} wm_state;

typedef struct {
    int x;
    int y;
    unsigned int width;
    unsigned int height;
    wm_state state;
	Window w;
} WmWindow;


typedef struct {
    int width;
    int height;
    int top;
    int left;
    int right;
    int bottom;
} WgGeometry ;


typedef struct Widget_s {
    Window w;
    struct Widget_s *parent;
    widget_type type;
    WgGeometry geom;
    int bmp;
    char *text;
    WmWindow *wm_window;
    void (*on_click)(struct Widget_s *,XButtonPressedEvent);
    void (*on_motion)(XMotionEvent);
	void (*on_unmap)(struct Widget_s *,XUnmapEvent);
} Widget ;


// prototypes
// **********

void connect_x_server();

void wg_resolve_geometry(WgGeometry *geom, Widget *parent, int *x,int *y, unsigned int *width, unsigned int *height);
Widget *create_widget(widget_type type,Widget *parent,WgGeometry *geometry,XColor color);
Widget *wg_create_from_x(widget_type,Window w,Widget *parent,WgGeometry *geometry);

void wg_move(Widget *wg,int new_x, int new_y);
void wg_resize(Widget *wg,unsigned int new_width, unsigned int new_height);

void draw_widget_button(Widget *wg);
void draw_widget_title_bar(Widget *wg);
void draw_widget_decoration(Widget *wg);

Widget *wg_find_from_window(Window w);

void create_window_decoration(Window window);

void on_click_title_bar(Widget *title_bar,XButtonPressedEvent e);
void on_click_decoration(Widget *decoration,XButtonPressedEvent e);
void on_click_close(Widget *button,XButtonPressedEvent e);
void on_click_full(Widget *button,XButtonPressedEvent e);
void on_click_iconify(Widget *button,XButtonPressedEvent e);

void on_motion_decoration(XMotionEvent e);

void on_unmap_xclient(Widget *xclient,XUnmapEvent e);

void wg_destroy_all();
void reparent_root_windows();

void main_event_loop();


#endif
