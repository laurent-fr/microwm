#ifndef widgets_h
#define widgets_h

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
    char **xpm;
    char *text;
    WmWindow *wm_window;
    void (*on_click)(struct Widget_s *,XButtonPressedEvent);
    void (*on_motion)(struct Widget_s *,XMotionEvent);
	void (*on_unmap)(struct Widget_s *,XUnmapEvent);
	void (*on_expose)(struct Widget_s *,XExposeEvent);
} Widget ;

// colors
enum {
    col_light,
    col_dark,
    col_normal,
    col_title,
    col_title_focus,
    col_count
};

int widget_cmp(const void *wg1,const void *wg2);

void wg_resolve_geometry(WgGeometry *geom, Widget *parent, int *x,int *y, unsigned int *width, unsigned int *height);
Widget *wg_find_from_window(Window w);

Widget *wg_create(widget_type type,Widget *parent,WgGeometry *geometry,XColor color);
Widget *wg_create_from_x(widget_type,Window w,Widget *parent,WgGeometry *geometry);

void wg_destroy(Widget *widget);

void wg_move(Widget *wg,int new_x, int new_y);
void wg_resize(Widget *wg,unsigned int new_width, unsigned int new_height);

void wg_destroy_all();

void draw_widget_button(Widget *wg,XExposeEvent e);
void draw_widget_title_bar(Widget *wg,XExposeEvent e);
void draw_widget_decoration(Widget *wg,XExposeEvent e);



#endif // widgets_h
