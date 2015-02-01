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

#ifndef widgets_h
#define widgets_h

#include <X11/Xlib.h> // Every Xlib program must include this
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h> // freetype
#include <X11/xpm.h>

#define NIL (0)       ///< A name for the void pointer

/// Types of widgets
typedef enum {
    wg_x11,         ///< a native X11 window
    wg_decoration,  ///< a window decoration (the frame around the X window)
    wg_title_bar,   ///< a title bar with a text at the center
    wg_button       ///< a button with an icon
}  widget_type ;

/// Window states
typedef enum {
    wm_normal,      ///< normal state
    wm_maximized,   ///< the window is maximized
    wm_icon         ///< the window is unmapped
} wm_state;

/// colors
typedef enum {
    xcol_light,
    xcol_dark,
    xcol_normal,
    xcol_count           ///< not a real color, gives the color count
} xcolors;

typedef enum {
    xftcol_normal,
    xftcol_title,
    xftcol_title_focus,
    xftcol_count
} xftcolors;

/// \brief Window Manager Window type
///
/// The x,y,whidth&height fields are used to save the position of the window
/// when it is maximized
///
typedef struct {
    int x;                  ///< the x position of the window
    int y;                  ///< the y position
    unsigned int width;     ///< the width
    unsigned int height;    ///< the height
    wm_state state;         ///< normal,maximezd,icon
	Window w;               ///< the X window which is decorated
} WmWindow;

/// \brief Geometry constraints for a widget
///
/// Unused parameters are set to -1
///
typedef struct {
    int width;              ///< fixed width of the widget
    int height;             ///< fixed height of the widget
    int top;                ///< top margin from the parent widget
    int left;               ///< left margin from the parent widget
    int right;              ///< right margin from the parent widget
    int bottom;             ///< bottom margin from the parent widget
} WgGeometry ;

/// \brief A gui widget
///
typedef struct Widget_s {
    Window w;                   ///< Underlying X window
    struct Widget_s *parent;    ///< parent widget
    widget_type type;           ///< widget type
    WgGeometry geom;            ///< geometry constraints
    char **xpm;                 ///< a pointer to a bitmap
    char *text;                 ///< a pointer to a text
    WmWindow *wm_window;        ///< a pointer to a WM window structure
    int fg_color;            ///< index to the fg color
    void (*on_click)(struct Widget_s *,XButtonPressedEvent);    ///< pointer to the function used on click event
    void (*on_motion)(struct Widget_s *,XMotionEvent);          ///< pointer to the function used on motion event
	void (*on_unmap)(struct Widget_s *,XUnmapEvent);            ///< pointer to the function used on unmap event
	void (*on_expose)(struct Widget_s *,XExposeEvent);          ///< pointer to the function used on expose event
} Widget ;



// protoypes
// **********

int widget_cmp(const void *wg1,const void *wg2);

void wg_resolve_geometry(WgGeometry *geom, Widget *parent, int *x,int *y, unsigned int *width, unsigned int *height);
Widget *wg_find_from_window(Window w);

Widget *wg_create(widget_type type,Widget *parent,WgGeometry *geometry,XColor color);
Widget *wg_create_from_x(widget_type,Window w,Widget *parent,WgGeometry *geometry);

void wg_free_widget(Widget *widget);
void wg_destroy(Widget *widget);

void wg_move(Widget *wg,int new_x, int new_y);
void wg_resize(Widget *wg,unsigned int new_width, unsigned int new_height);

void wg_destroy_all();

void draw_shadow(Window w,GC gc,int x1,int y1,int x2,int y2,XColor nw,XColor se);

void draw_widget_button(Widget *wg,XExposeEvent e);
void draw_widget_title_bar(Widget *wg,XExposeEvent e);
void draw_widget_decoration(Widget *wg,XExposeEvent e);

#endif // widgets_h
