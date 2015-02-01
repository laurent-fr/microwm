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

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "widgets.h"
#include "icccm.h"
#include "microwm.h"

#include "bitmap/close.xpm"
#include "bitmap/full.xpm"
#include "bitmap/unfull.xpm"
#include "bitmap/iconify.xpm"

// globals
// ****

// the config array
extern ConfigElement _config[];

// X11
Display *display;   ///< global variable, the X display
int screen_num;     ///< global variable, the X screen number
Colormap colormap;  ///< global variable, the X color map

// colors
XColor _xcolors[xcol_count];  ///< global array, the X colors available to the application
XftColor _xftcolors[xftcol_count];

//char *colors_text[] = {"#f4f4f4","#978d8d","#dbdbdb","#333333","#d06610"};

// cursors
#define NB_CURSOR 11
unsigned int cursors_def[]={
		XC_left_ptr,XC_top_side,XC_bottom_side,XC_fleur,
		XC_right_side,XC_top_right_corner,XC_bottom_right_corner, XC_arrow,
		XC_left_side,XC_top_left_corner,XC_bottom_left_corner
		 };
Cursor xcursors[NB_CURSOR]; ///< global array, the X cursors availables to the application

// xft font
XftFont *_xft_font; ///< global variable, the Xft font used for drawing titles

// Focused window
Widget *_focused = NULL;

// X11 functions
// **************

void allocate_xcolor(char *color_text,XColor *xcolor) {
    XParseColor(display, colormap,color_text,xcolor);
    XAllocColor(display, colormap, xcolor);
}

void allocate_xftcolor(char *color_text,XftColor *xftcolor) {
            XftColorAllocName(display,DefaultVisual(display,0),DefaultColormap(display,0),color_text,xftcolor);
}

/// \brief Init X connection
///
/// Open X Display, allocate X colors, X cursors & Xft font
///
void connect_x_server() {

    display = XOpenDisplay(NIL);

    screen_num = DefaultScreen(display);
    colormap = XDefaultColormap(display,screen_num);

    // allocate X colors
    allocate_xcolor(_config[cfg_col_light].string,&_xcolors[xcol_light]);
    allocate_xcolor(_config[cfg_col_dark].string,&_xcolors[xcol_dark]);
    allocate_xcolor(_config[cfg_col_normal].string,&_xcolors[xcol_normal]);

    // allocate Xft colors
   allocate_xftcolor(_config[cfg_title_bar_font_color_focus].string,&_xftcolors[xftcol_title_focus]);
   allocate_xftcolor(_config[cfg_title_bar_font_color_unfocus].string,&_xftcolors[xftcol_title]);
   allocate_xftcolor(_config[cfg_col_normal].string,&_xftcolors[xftcol_normal]);

    // init cursors
    for(int i=0;i<NB_CURSOR;i ++) {
		xcursors[i]=XCreateFontCursor(display,cursors_def[i]);
	}

	// open xft font
	int font_weight = XFT_WEIGHT_MEDIUM;
	if (!strcmp(_config[cfg_title_bar_font_weight].string,"bold")) font_weight = XFT_WEIGHT_BOLD;


    _xft_font =  XftFontOpen (display, screen_num,
                             XFT_FAMILY, XftTypeString, _config[cfg_title_bar_font_name].string ,
                             XFT_SIZE, XftTypeDouble, (double)_config[cfg_title_bar_font_size].number,
                            XFT_WEIGHT,XftTypeInteger,font_weight,
                             NULL);

}

/// \brief Close X connection
///
/// Free Xft font, cleanup widgets
///
void disconnect_x_server() {

    // free xft font
    XftFontClose(display,_xft_font);

    // TODO : free Xft colors

    // destroy all widgets
    //wg_destroy_all();

}

// WM functions
// *************

/// \brief Create the window decoration around a X window
///
/// \param window a X window
///
/// The decoration is made of multiple widgets :
/// * A decoration widget which contains a title bar and the X window
/// * A title bar widget which contains 3 buttons
/// * A button widget for closing the window
/// * A button widget for iconify
/// * A button widget for maximizing
///
/// At the and of the process the X window is reparented to the decoration
/// and all windows are mapped on screen.
///
void create_window_decoration(Window window) {

    // change window border
    XWindowChanges changes;
    changes.border_width=0;
    XConfigureWindow(display,window,CWBorderWidth,&changes);

    // get  window size
    XWindowAttributes deco_attrs;
    XGetWindowAttributes(display,window,&deco_attrs);

    // create decoration
    int deco_x=deco_attrs.x-DECORATION_MARGIN;
    int deco_y=deco_attrs.y-DECORATION_MARGIN_TOP;
    int deco_w=deco_attrs.width+DECORATION_MARGIN*2;
    int deco_h=deco_attrs.height+DECORATION_MARGIN+DECORATION_MARGIN_TOP;

    if(deco_x<0) deco_x=0;
    if(deco_y<0) deco_y=0;

    // create WmWindow
    WmWindow *wm_window = (WmWindow *)malloc(sizeof(WmWindow));
    wm_window->state=wm_normal;
    wm_window->x=deco_x;
    wm_window->y=deco_y;
    wm_window->width=deco_w;
    wm_window->height=deco_h;
	wm_window->w = window;

    // add decoration frame
    WgGeometry frame_geom = { .left=deco_x, .top=deco_y, .width=deco_w, .height=deco_h , .bottom=-1, .right=-1 };

    Widget *decoration = wg_create(wg_decoration, NULL,
                                       &frame_geom,_xcolors[xcol_normal]);

    decoration->on_motion = &on_motion_decoration;
    decoration->on_click = &on_click_decoration;
    decoration->wm_window = wm_window;

    // add title bar
    int title_height=DECORATION_MARGIN_TOP-DECORATION_MARGIN;
    WgGeometry title_bar_geom = { .left=DECORATION_MARGIN, .top=DECORATION_MARGIN-1, .width=-1, .height=title_height, .bottom=-1, .right=DECORATION_MARGIN };
    Widget *title_bar = wg_create(wg_title_bar, decoration,
                                      &title_bar_geom,_xcolors[xcol_normal]);

    title_bar->fg_color = xftcol_title;

    title_bar->on_click = &on_click_title_bar;
    title_bar->on_expose = &paint_title_bar;
    get_window_name(window,&(title_bar->text));

    // add close button
    int button_width=title_height-1;
    WgGeometry close_geom = { .left=0, .top=0, .width=button_width, .height=button_width, .bottom=-1, .right=-1 };
    Widget *close_button = wg_create(wg_button,title_bar,
                                         &close_geom,_xcolors[xcol_normal]);

    close_button->xpm = close_xpm;
    close_button->on_click = &on_click_close;

    // add maximize button
    WgGeometry full_geom = { .left=-1, .top=0, .width=button_width, .height=button_width, .bottom=-1, .right=0 };
    Widget *full_button = wg_create(wg_button,title_bar,
                                        &full_geom,_xcolors[xcol_normal]);

    full_button->xpm = full_xpm;
    full_button->on_click = &on_click_full;
    full_button->on_expose = &paint_full_button;

    // add iconify button
    WgGeometry iconify_geom = { .left=-1, .top=0, .width=button_width, .height=button_width, .bottom=-1, .right=button_width-1};
    Widget *iconify_button = wg_create(wg_button,title_bar,
                                           &iconify_geom,_xcolors[xcol_normal]);

    iconify_button->xpm = iconify_xpm;
    iconify_button->on_click = &on_click_iconify;

    // the x11 window itself
    WgGeometry window_geom = { .left=DECORATION_MARGIN, .top=DECORATION_MARGIN_TOP, .width=-1, .height=-1, .bottom=DECORATION_MARGIN, .right=DECORATION_MARGIN };
    Widget *xclient = wg_create_from_x(wg_x11,window,decoration,&window_geom);

    xclient->on_unmap = &on_unmap_xclient;

    XSelectInput(display, xclient->w ,  SubstructureRedirectMask );

    // add to SaveSet
    XAddToSaveSet(display,window);

    // reparent window into decoration
    XReparentWindow(display,window,decoration->w,DECORATION_MARGIN,DECORATION_MARGIN_TOP);

    // display windows
    XMapWindow(display,decoration->w);
    XMapSubwindows(display,decoration->w);
    XMapSubwindows(display,title_bar->w);

    XFlush(display);

}

/// \brief focus a window
///
/// \param decoration the decoration widget
///
void focus_wm_window(Widget *decoration) {

    // raise the window
    XRaiseWindow(display, decoration->w);

    // get input focus
    XSetInputFocus(display, decoration->wm_window->w, RevertToPointerRoot,CurrentTime);

    // TODO : redraw old and new title bar

    _focused = decoration ;

}

// Paint events
// *************

/// \brief paint the maximize button
///
/// \param button the button widget
/// \param e the XExposeEvent
///
/// Change the icon according to the state of the window (maximized/normal)
/// then call the original paint function
///
void paint_full_button(Widget *button,XExposeEvent e) {

    Widget *decoration = button->parent->parent;
    if (decoration->wm_window->state == wm_maximized)
        button->xpm = unfull_xpm;
    else
        button->xpm = full_xpm;

    draw_widget_button(button,e);

}

void paint_title_bar(Widget *title_bar,XExposeEvent e) {

      Widget *decoration =  title_bar->parent;

      if (decoration==_focused)
        title_bar->fg_color = xftcol_title_focus;
      else
        title_bar->fg_color = xftcol_title;

      draw_widget_title_bar(title_bar,e);

}


// Widget events
// **************

/// \brief display one of 8 resize cursors when the mouse is on the edge of a decoration window
///
/// \param decoration the widget in which the mouse is moving
/// \param e the XMotionEvent
///
/// This function does nothing if the window is maximized.
///
void on_motion_decoration(Widget *decoration, XMotionEvent e) {

    // do nothing if windows is maximized
    if (decoration->wm_window->state == wm_maximized ) return;

    int x = e.x;
    int y = e.y;
    int position = 0;

    //get decoration size
    XWindowAttributes deco_attrs;
    XGetWindowAttributes(display,e.window,&deco_attrs);

    if (x<DECORATION_MARGIN_TOP) position |= west;
    if (x>(deco_attrs.width-DECORATION_MARGIN_TOP)) position |= east;
    if (y<DECORATION_MARGIN_TOP) position |= north;
    if (y>(deco_attrs.height-DECORATION_MARGIN_TOP)) position |= south;

    XDefineCursor(display, e.window, xcursors[position]);

}

/// \brief Destroy a decoration when the X Window is unmapped
///
/// \param xclient the widget which contains the X window
/// \param e the XUnmapEvent
///
/// \todo reparent the X window do root, remove frome save set
///
void on_unmap_xclient(Widget *xclient,XUnmapEvent e) {

	Widget *decoration = xclient->parent;
	if (!decoration) { printf("No decoration\n"); return; }

	if (e.event == DefaultRootWindow(display))
        return;

    #ifdef DEBUG
        printf("Unmap xclient %d %d\n",(int)e.window,(int)decoration->w);
    #endif

    XGrabServer(display);
    // TODO : check if the window still exists

    //XUnmapWindow(display,e.window);
    //XReparentWindow(display,e.window,RootWindow(display, screen_num),0,0 );
    // XRemoveFromSaveSet(display,e.window);

    wg_destroy(decoration);

    wg_free_widget(xclient); // TODO : call only if the x window does'nt exists

    XUngrabServer(display);
    XFlush(display);

}

/// \brief Move a window when the user drag the title bar
///
/// \param title_bar the title bar which is clicked
/// \param e the XButtonPressedEvent
///
/// A simple click focus the window
///
void on_click_title_bar(Widget *title_bar,XButtonPressedEvent e) {

    // raise window
    Widget *decoration = title_bar->parent;
    focus_wm_window(decoration);

    // get initial mouse_position
    int x_mouse_init = e.x_root;
    int y_mouse_init = e.y_root;

    // get initial decoration window position
    XWindowAttributes window_init_attrs;
    XGetWindowAttributes(display,decoration->w,&window_init_attrs);
    int x_window_init = window_init_attrs.x;
    int y_window_init = window_init_attrs.y;

    // sub event-loop, exits when button mouse is released
    XEvent event;
    Bool moving = True;
    int x_mouse_current,y_mouse_current,new_x,new_y;
    while (moving) {
        XNextEvent(display, &event);

        switch (event.type) {
            case ButtonRelease:
                moving = False;
                break;

            case MotionNotify:

                // discard other motion events
                while (XCheckTypedEvent(display, MotionNotify, &event));

                 // if windows is maximized, set it to normal and update width an height
                if (decoration->wm_window->state == wm_maximized ) {
                    decoration->wm_window->state = wm_normal;
                    wg_resize(decoration,decoration->wm_window->width,decoration->wm_window->height );
                    // if mouse is out of the title_bar, move it to the right
                    if (((unsigned int)x_mouse_init-(unsigned int)x_window_init)>(decoration->wm_window->width-2*DECORATION_MARGIN_TOP)) {
                        x_window_init+=x_mouse_init-x_window_init-decoration->wm_window->width+2*DECORATION_MARGIN_TOP;
                        wg_move(decoration,x_window_init,y_window_init);
                    }
                }

                x_mouse_current = event.xmotion.x_root;
                y_mouse_current = event.xmotion.y_root;

                new_x = x_window_init + x_mouse_current - x_mouse_init;
                new_y = y_window_init + y_mouse_current - y_mouse_init;

                //  minimum  values
                if (new_x<0) new_x=0;
                if (new_y<0) new_y=0;

                // move the window
                wg_move(title_bar->parent,new_x,new_y);

                break;

            case Expose:
                on_expose_event(event.xexpose);
                break;
        }

    }

}

/// \brief Resize a window when the user drag the border of the decoration
///
/// \param decoration the decoration which is clicked
/// \param e the XButtonPressedEvent
///
/// A simple click focus the window
void on_click_decoration(Widget *decoration,XButtonPressedEvent e) {

    // do nothing if window is maximized
    if (decoration->wm_window->state == wm_maximized ) return;

    focus_wm_window(decoration);

    // get initial mouse_position
    int x_mouse_init = e.x_root;
    int y_mouse_init = e.y_root;

    // get initial decoration window position
    XWindowAttributes window_init_attrs;
    XGetWindowAttributes(display,decoration->w,&window_init_attrs);
    int width_window_init = window_init_attrs.width;
    int height_window_init = window_init_attrs.height;
    int x_window_init = window_init_attrs.x;
    int y_window_init = window_init_attrs.y;

    // get resize type
    int position = 0;
    if (e.x<DECORATION_MARGIN_TOP) position |= west;
    if (e.x>(width_window_init-DECORATION_MARGIN_TOP)) position |= east;
    if (e.y<DECORATION_MARGIN_TOP) position |= north;
    if (e.y>(height_window_init-DECORATION_MARGIN_TOP)) position |= south;

    // get hints from window
    icccm_size_hints hints;
    get_wm_normal_hints(decoration->wm_window->w,&hints);
    printf("min_width=%d, max_width=%d, width_inc=%d\nmin_height=%d, max_height=%d, height_inc=%d\n",
           hints.min_width,hints.max_width,hints.width_inc,
           hints.min_height,hints.max_height,hints.height_inc);

    // sub event-loop, exits when button mouse is released
    XEvent event;
    Bool resizing = True;
    int x_mouse_current,y_mouse_current,new_x,new_y,new_width,new_height,resize_dir_x,resize_dir_y;
    while (resizing) {
        XNextEvent(display, &event);

        switch (event.type) {
            case ButtonRelease:
                resizing = False;
                break;

            case MotionNotify:

                // discard other motion events
                while (XCheckTypedEvent(display, MotionNotify, &event));

                x_mouse_current = event.xmotion.x_root;
                y_mouse_current = event.xmotion.y_root;

                new_width = width_window_init;
                new_height = height_window_init;
                new_x = x_window_init;
                new_y = y_window_init;
                resize_dir_x= 1;
                resize_dir_y= 1;

                // calculate the new location and size of the window
                if (position&north) { new_y+= y_mouse_current - y_mouse_init; resize_dir_y=-1; }
                if (position&west) { new_x+= x_mouse_current - x_mouse_init; resize_dir_x=-1; }
                if (position&(north|south)) new_height+=(y_mouse_current - y_mouse_init)*resize_dir_y;
                if (position&(west|east)) new_width+=(x_mouse_current - x_mouse_init)*resize_dir_x;

                //  minimum  values
                if (new_x<0) new_x=0;
                if (new_y<0) new_y=0;
                if ((hints.min_width>0)&&(new_width<hints.min_width)) new_width=hints.min_width;
                if ((hints.min_height>0)&&(new_height<hints.min_height)) new_height=hints.min_height;
                if (new_width<=MIN_WINDOW_WIDTH) new_width=MIN_WINDOW_WIDTH;
                if (new_height<=MIN_WINDOW_HEIGHT) new_height=MIN_WINDOW_HEIGHT;

                // maximum values
                if ((hints.max_width>0)&&(new_width>hints.max_width)) new_width=hints.max_width;
                if ((hints.max_height>0)&&(new_height>hints.max_height)) new_height=hints.max_height;

                // increment
                if (hints.width_inc>0) new_width = (new_width/hints.width_inc)*hints.width_inc;
                if (hints.height_inc>0) new_height = (new_height/hints.height_inc)*hints.height_inc;

                // move and resize if needed
                if ((new_width!=width_window_init)||(new_height!=height_window_init)) wg_resize(decoration, new_width, new_height);
                if ((new_x!=x_window_init)||(new_y!=y_window_init)) wg_move(decoration, new_x, new_y);

                break;

            case Expose:
                on_expose_event(event.xexpose);
                break;
        }

    }
}

/// \brief Close window action
///
/// \param button the close button
/// \param e the XButtonPressedEvent
///
/// If supported, send a WM_DELETE_WINDOW to the client
/// otherwise kill the client
void on_click_close(Widget *button,XButtonPressedEvent e  __attribute__((__unused__)) ) {

    Window window = button->parent->parent->wm_window->w;

	// send WM_DELETE_WINDOW to X client
	XEvent ev;
	memset(&ev, 0, sizeof (ev));

    if (has_wm_delete_window(window)==True) {
        ev.xclient.type = ClientMessage;
        ev.xclient.window = window;
        ev.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", True);
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW", False);
        XSendEvent(display, window, False, NoEventMask, &ev);
    } else {
        XKillClient(display,window);
    }

}

/// \brief Iconify window action
///
/// \param button the close button
/// \param e the XButtonPressedEvent
///
/// \todo implement the function ....
///
void on_click_iconify(Widget *button  __attribute__((__unused__)),XButtonPressedEvent e  __attribute__((__unused__)) ) {

    printf("Not implemented yet.\n");
}

/// \brief Maximize window action
///
/// \param button the close button
/// \param e the XButtonPressedEvent
///
/// When maximizing, the old position of the window is stored in decoration->wm_window
///
void on_click_full(Widget *button,XButtonPressedEvent e  __attribute__((__unused__)) ) {

    Widget *decoration = button->parent->parent;
    if (!decoration) return;
	WmWindow *wm_window = decoration->wm_window;

	// focus window
    focus_wm_window(decoration);

	// maximized --> normal
	if (wm_window->state == wm_maximized) {

		wg_resize(decoration,wm_window->width,wm_window->height);
		wg_move(decoration,wm_window->x,wm_window->y);

		wm_window->state = wm_normal;

		return;
	}

	// normal --> maximized
	if (wm_window->state == wm_normal) {
		// get root window size
		Window root = RootWindow(display, screen_num);
	    XWindowAttributes root_attrs;
	    XGetWindowAttributes(display,root,&root_attrs);

		// get decoration window size
		Window decoration = button->parent->parent->w;
	    XWindowAttributes deco_attrs;
	    XGetWindowAttributes(display,decoration,&deco_attrs);

		wm_window->x = deco_attrs.x;
		wm_window->y = deco_attrs.y;
		wm_window->width = deco_attrs.width;
		wm_window->height = deco_attrs.height;

		wg_move(button->parent->parent,root_attrs.x,root_attrs.y);
		wg_resize(button->parent->parent,root_attrs.width,root_attrs.height);

		wm_window->state = wm_maximized;

		return;
	}


}

// generic events handlers
// ************************

/// \brief Handle expose event
///
/// \param e the XExposeEvent
///
void on_expose_event(XExposeEvent e) {

    Widget *widget = wg_find_from_window(e.window);
    if (!widget) return;

    if (widget->on_expose) widget->on_expose(widget,e);
}

/// \brief Handle button press event
///
/// \param e the XButtonPressedEvent
///
void on_buttonpress_event(XButtonPressedEvent e) {

    Widget *widget = wg_find_from_window(e.window);
    if (!widget) return;

    if (widget->on_click) widget->on_click(widget,e);
}

/// \brief Handle motion event
///
/// \param e the XMotionEvent
///
void on_motion_event(XMotionEvent e) {

    Widget *widget = wg_find_from_window(e.window);
    if (!widget) return;

    if (widget->on_motion) widget->on_motion(widget,e);
}

/// \brief Handle unmap event
///
/// \param e the XUnmapEvent
///
void on_unmap_event(XUnmapEvent e) {

	Widget *widget = wg_find_from_window(e.window);
    if (!widget) return;

    #ifdef DEBUG
        printf("on_unmap_event e.w=%d, widget->=%d\n",(int)e.window,(int)widget->w);
    #endif

	if (widget->on_unmap) widget->on_unmap(widget,e);
}

/// \brief Handle configure request event
///
/// \param e the XConfigureRequestEvent
///
/// \todo not very useful for now
///
void on_configure_request(XConfigureRequestEvent e) {

    XWindowChanges changes;

    changes.x = e.x;
    changes.y = e.y;
    changes.width = e.width;
    changes.height = e.height;
    changes.border_width=0;

    XConfigureWindow(display,e.window, CWX|CWY|CWWidth| CWHeight|CWBorderWidth ,&changes);

}

void on_configure_notify(XConfigureEvent e) {

    #ifdef DEBUG
        printf("Configure notify %d\n",(int)e.window);
    #endif // DEBUG

	Widget *widget = wg_find_from_window(e.window);
    if (!widget) return;

    printf("type %d\n",widget->type);

    printf("x=%d y=%d w=%d h=%d\n",e.x,e.y,e.width,e.height);

}

/// \brief Reparent all X windows without decoration
///
/// Used at startup
///
void reparent_root_windows() {

    // grap display during reparenting
    XGrabServer(display);

    //find root windows
    Window root_return,parent;
    Window *children,*child;
    unsigned int nchildren=0,i;
    Window root = RootWindow(display, screen_num);
    XQueryTree(display, root, &root_return, &parent, &children, &nchildren);

    // reparent windows
    for(i=0, child=children; i<nchildren; i ++,child++) {
        create_window_decoration(*child);
    }

    if (children) XFree(children);

    // ungrap display
    XUngrabServer(display);

    XFlush(display);

}

/// \brief the main event loop
///
/// waits and dispatch the X events
///
void main_event_loop() {

    XEvent event;

    // select events
    Window root = RootWindow(display, screen_num);
    XSelectInput(display, root , SubstructureRedirectMask | SubstructureNotifyMask );

    while (1) {
        /* Wait for an event */
        XNextEvent(display, &event);

        switch (event.type) {
        case MapNotify:
           // if (event.xmap.override_redirect == False) {
                printf("MapNotify %d\n",event.xmap.override_redirect);
            //    create_window_decoration(event.xmap.window);
          // }
            break;

        case MapRequest:
            printf("Map Request\n");
            create_window_decoration(event.xmaprequest.window);
            break;

        case ConfigureRequest:
            on_configure_request(event.xconfigurerequest);
            break;

        case ConfigureNotify:
            on_configure_notify(event.xconfigure);
        break;

        case Expose:
            on_expose_event(event.xexpose);
            break;

		case UnmapNotify:
			printf("Unmap event\n");
			on_unmap_event(event.xunmap);
			break;

		case DestroyNotify:
			printf("Destroy event \n");
			break;

        case MotionNotify:
            // discard other motion events
            while (XCheckTypedEvent(display, MotionNotify, &event));
            on_motion_event(event.xmotion);
            break;

        case ButtonPress:
            printf("Button Press event\n");
            on_buttonpress_event(event.xbutton);
            break;

        default:
            printf("Unhandled event %d\n",event.type);

        }
    }


}
