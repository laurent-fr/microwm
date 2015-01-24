#include <stdio.h>
#include <stdlib.h>
#include <search.h>

#include "widgets.h"
#include "microwm.h"
#include "bitmap/close.xpm"
#include "bitmap/full.xpm"
#include "bitmap/unfull.xpm"
#include "bitmap/iconify.xpm"

// globals
Display *display;
int screen_num;
Colormap colormap;



XColor xcolors[col_count];
char *colors_text[] = {"#f4f4f4","#978d8d","#dbdbdb","#333333"};

// cursors
#define NB_CURSOR 11
unsigned int cursors_def[]={
		XC_left_ptr,XC_top_side,XC_bottom_side,XC_fleur,
		XC_right_side,XC_top_right_corner,XC_bottom_right_corner, XC_arrow,
		XC_left_side,XC_top_left_corner,XC_bottom_left_corner
		 };

Cursor xcursors[NB_CURSOR];


void connect_x_server() {

    display = XOpenDisplay(NIL);
    // TODO : fail if no display

    screen_num = DefaultScreen(display);
    colormap = XDefaultColormap(display,screen_num);

    // allocate colors
    for (int i=0; i<col_count; i++) {
        XParseColor(display,colormap,colors_text[i],&xcolors[i]);
        XAllocColor(display, colormap, &xcolors[i]);
    }

    // init cursors
    for(int i=0;i<NB_CURSOR;i ++) {
		xcursors[i]=XCreateFontCursor(display,cursors_def[i]);
	}


}



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

    // decoration frame
    WgGeometry frame_geom = { .left=deco_x, .top=deco_y, .width=deco_w, .height=deco_h , .bottom=-1, .right=-1 };

    Widget *decoration = create_widget(wg_decoration, NULL,
                                       &frame_geom,xcolors[col_normal]);


    // add title bar
    int title_height=DECORATION_MARGIN_TOP-DECORATION_MARGIN;
    WgGeometry title_bar_geom = { .left=DECORATION_MARGIN, .top=DECORATION_MARGIN-1, .width=-1, .height=title_height, .bottom=-1, .right=DECORATION_MARGIN };
    Widget *title_bar = create_widget(wg_title_bar, decoration,
                                      &title_bar_geom,xcolors[col_normal]);

    // get window title
    get_window_name(window,&(title_bar->text));
    printf("name=%s\n",title_bar->text);

    // add buttons
    int button_width=title_height-1;
    WgGeometry close_geom = { .left=0, .top=0, .width=button_width, .height=button_width, .bottom=-1, .right=-1 };
    Widget *close_button = create_widget(wg_button,title_bar,
                                         &close_geom,xcolors[col_normal]);

    close_button->xpm = close_xpm;

    WgGeometry full_geom = { .left=-1, .top=0, .width=button_width, .height=button_width, .bottom=-1, .right=0 };
    Widget *full_button = create_widget(wg_button,title_bar,
                                        &full_geom,xcolors[col_normal]);
    full_button->xpm = full_xpm;

    WgGeometry iconify_geom = { .left=-1, .top=0, .width=button_width, .height=button_width, .bottom=-1, .right=button_width-1};
    Widget *iconify_button = create_widget(wg_button,title_bar,
                                           &iconify_geom,xcolors[col_normal]);
    iconify_button->xpm = iconify_xpm;

    // the x11 window itself
    WgGeometry window_geom = { .left=DECORATION_MARGIN, .top=DECORATION_MARGIN_TOP, .width=-1, .height=-1, .bottom=DECORATION_MARGIN, .right=DECORATION_MARGIN };
    Widget *xclient = wg_create_from_x(wg_x11,window,decoration,&window_geom);

    // add events
    decoration->on_motion = &on_motion_decoration;
    decoration->on_click = &on_click_decoration;
    title_bar->on_click = &on_click_title_bar;
    close_button->on_click = &on_click_close;
    iconify_button->on_click = &on_click_iconify;
    full_button->on_click = &on_click_full;
	xclient->on_unmap = &on_unmap_xclient;

    // link to WmWindow
    decoration->wm_window = wm_window;


    // Add to SaveSet
    XAddToSaveSet(display,window);

    // reparent window into decoration
    XReparentWindow(display,window,decoration->w,DECORATION_MARGIN,DECORATION_MARGIN_TOP);

    XFlush(display);

}




void on_expose_event(XExposeEvent e) {

    // find widget in widget list
    Widget *widget = wg_find_from_window(e.window);
    if (!widget) return;

    switch(widget->type) {
    case wg_decoration:
        draw_widget_decoration(widget);
        //printf("decoration\n");

        break;

    case wg_title_bar:
        draw_widget_title_bar(widget);
        //printf("title_bar\n");

        break;

    case wg_button:
        draw_widget_button(widget);
        //printf("title_bar\n");

        break;


    default:

        break;
    }

}


// Widget events
// **************

// display one of 8 resize cursors when the mouse is on the edge of a decoration window
void on_motion_decoration(XMotionEvent e) {

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


// destroy decoration
void on_unmap_xclient(Widget *decoration,XUnmapEvent e) {
	printf("Unmap xclient\n");
}

// move a window
void on_click_title_bar(Widget *title_bar,XButtonPressedEvent e) {

    // raise window
    XRaiseWindow(display, title_bar->parent->w);

    // get initial mouse_position
    int x_mouse_init = e.x_root;
    int y_mouse_init = e.y_root;

    // get initial decoration window position
    XWindowAttributes window_init_attrs;
    XGetWindowAttributes(display,title_bar->parent->w,&window_init_attrs);
    int x_window_init = window_init_attrs.x;
    int y_window_init = window_init_attrs.y;

    // sub event-loop, exits when button mouse is released
    XEvent event;
    Bool moving = True;
    int x_mouse_current,y_mouse_current;
    while (moving) {
        XNextEvent(display, &event);

        switch (event.type) {
            case ButtonRelease:
                moving = False;
                break;

            case MotionNotify:
                x_mouse_current = event.xmotion.x_root;
                y_mouse_current = event.xmotion.y_root;

                wg_move(title_bar->parent,x_window_init + x_mouse_current - x_mouse_init ,y_window_init + y_mouse_current - y_mouse_init);

                while (XCheckTypedEvent(display, MotionNotify, &event));

                break;

            case Expose:
                on_expose_event(event.xexpose);
                break;
        }

    }

}

// resize a window
void on_click_decoration(Widget *decoration,XButtonPressedEvent e) {

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

                //  minimum width and height
                if (new_width<=2*DECORATION_MARGIN+16) new_width=2*DECORATION_MARGIN+16;
                if (new_height<=DECORATION_MARGIN+DECORATION_MARGIN_TOP+16) new_height=DECORATION_MARGIN+DECORATION_MARGIN_TOP+16;

                // move and resize if needed
                if ((new_width!=width_window_init)||(new_height!=height_window_init)) wg_resize(decoration, new_width, new_height);
                if ((new_x!=x_window_init)||(new_y!=y_window_init)) wg_move(decoration, new_x, new_y);

                while (XCheckTypedEvent(display, MotionNotify, &event));

                break;

            case Expose:
                on_expose_event(event.xexpose);
                break;
        }

    }
}

void on_click_close(Widget *button,XButtonPressedEvent e) {

    Window window = button->parent->parent->wm_window->w;

	// send WM_DELETE_WINDOW to X client
	XEvent ev;
	memset(&ev, 0, sizeof (ev));

	ev.xclient.type = ClientMessage;
	ev.xclient.window = window;
	ev.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", True);
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW", False);
	ev.xclient.data.l[1] = CurrentTime;
	XSendEvent(display, window, False, NoEventMask, &ev);


	// TODO : call XKillClient if the client is still there.


}

void on_click_iconify(Widget *button,XButtonPressedEvent e) {


    printf("Not implemented yet.\n");
}

void on_click_full(Widget *button,XButtonPressedEvent e) {

	WmWindow *wm_window = button->parent->parent->wm_window;
	if (!wm_window) return;

	// raise window
    XRaiseWindow(display, button->parent->parent->w);

	// maximized to normal window
	if (wm_window->state == wm_maximized) {

        button->xpm = full_xpm;

		wg_resize(button->parent->parent,wm_window->width,wm_window->height);
		wg_move(button->parent->parent,wm_window->x,wm_window->y);

		wm_window->state = wm_normal;

		return;
	}

	// normal to maximized window
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

        button->xpm = unfull_xpm ;

		wg_move(button->parent->parent,root_attrs.x,root_attrs.y);
		wg_resize(button->parent->parent,root_attrs.width,root_attrs.height);

		wm_window->state = wm_maximized;

		return;
	}


}

// generic events handlers
// ************************

void on_buttonpress_event(XButtonPressedEvent e) {

    Widget *widget = wg_find_from_window(e.window);
    if (!widget) return;

    if (widget->on_click) widget->on_click(widget,e);
}

void on_motion_event(XMotionEvent e) {

    Widget *widget = wg_find_from_window(e.window);
    if (!widget) return;

    if (widget->on_motion) widget->on_motion(e);
}

void on_unmap_event(XUnmapEvent e) {

	Widget *widget = wg_find_from_window(e.window);
    if (!widget) return;

	if (widget->on_unmap) widget->on_unmap(widget,e);
}


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

}



void main_event_loop() {

    XEvent event;

    // select events
    Window root = RootWindow(display, screen_num);
    XSelectInput(display, root ,
                 SubstructureRedirectMask |SubstructureNotifyMask);


    while (1) {
        /* Wait for an event */
        XNextEvent(display, &event);

        switch (event.type) {
        case MapNotify:
            if (event.xmap.override_redirect == False) {
                printf("MapNotify %d\n",event.xmap.override_redirect);
                create_window_decoration(event.xmap.window);
           }
            break;

        case Expose:
            on_expose_event(event.xexpose);
            break;

		case UnmapNotify:
			printf("Unmap event\n");
			on_unmap_event(event.xunmap);
			break;

		case DestroyNotify:
			printf("Destroy event\n");
			break;

        case MotionNotify:
            on_motion_event(event.xmotion);
            break;

        case ButtonPress:
            printf("Button Press event\n");
            on_buttonpress_event(event.xbutton);
            break;

        }
    }


}
