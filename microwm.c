#include <stdio.h>
#include <stdlib.h>
#include <search.h>

#include "microwm.h"
#include "bitmap/close.xpm"
#include "bitmap/full.xpm"
#include "bitmap/iconify.xpm"

Display *display;
int screen_num;
Colormap colormap;

enum {
    col_light,
    col_dark,
    col_normal,
    col_title,
    col_count
};

XColor xcolors[col_count];

char *colors_text[] = {"#f4f4f4","#978d8d","#dbdbdb","#333333"};

void *widget_list = NULL;


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

}

int widget_cmp(const void *wg1,const void *wg2) {
    Widget *wwg1=(Widget *)wg1;
    Widget *wwg2=(Widget *)wg2;
    return wwg1->w - wwg2->w;
}


void wg_resolve_geometry(WgGeometry *geom, Widget *parent, int *x,int *y, unsigned int *width, unsigned int *height) {

    *x=0;
    *y=0;
    *width=0;
    *height=0;

    if (geom->left>=0) *x=geom->left;
    if (geom->top>=0) *y=geom->top;
    if (geom->width>=0) *width=geom->width;
    if (geom->height>=0) *height=geom->height;

    if (geom->bottom>=0 || geom->right>=0) {

        XWindowAttributes parent_attrs;
        XGetWindowAttributes(display,parent->w,&parent_attrs);

        if ((geom->right>=0)&&(geom->width>0)) *x = parent_attrs.width - geom->width - geom->right;
        if ((geom->bottom>=0)&&(geom->height>0)) *y = parent_attrs.height - -geom->height - geom->bottom;
        if (geom->width<0) *width = parent_attrs.width - geom->left - geom->right;
        if (geom->height<0) *height = parent_attrs.height - geom->top - geom->bottom;

    }

    //printf("resolv geom    : l=%d, t=%d, w=%d, h=%d, r=%d, b=%d\n",geom->left,geom->top,geom->width,geom->height,geom->right,geom->bottom);
    //printf("resolve result : x=%d, y=%d, w=%d, h=%d\n",*x,*y, *width, *height);
}



Widget *create_widget(widget_type type,Widget *parent,WgGeometry *geometry,XColor color) {

    int x,y;
    unsigned int width,height;

    wg_resolve_geometry(geometry,parent,&x,&y,&width,&height);
    printf("size = %d %d %d %d\n",x,y,width,height);
    // create window
    Window parent_window;
    if (parent==NULL)
        parent_window=DefaultRootWindow(display);
    else
        parent_window=parent->w;

    Window w = XCreateSimpleWindow(display, parent_window,x,y,width,height, 0, NIL, color.pixel);

    Widget *new_widget =  wg_create_from_x(type,w,parent,geometry);

    // add event
    long event_mask = 0;
    switch(type) {
        case wg_decoration:
            event_mask =  ExposureMask | EnterWindowMask | LeaveWindowMask | ButtonPressMask | ButtonReleaseMask |PointerMotionMask;
            break;
        case wg_title_bar:
            event_mask =  ExposureMask | EnterWindowMask | LeaveWindowMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
            break;
        case wg_button:
            event_mask =  ExposureMask | EnterWindowMask | LeaveWindowMask | ButtonPressMask;
            break;
        default:
            event_mask =  ExposureMask | EnterWindowMask | LeaveWindowMask | ButtonPressMask;

    }
    XSelectInput(display, new_widget->w , event_mask );

    // map window
    XMapWindow(display, new_widget->w);

    return new_widget;
}


Widget *wg_create_from_x(widget_type type,Window w,Widget *parent,WgGeometry *geometry) {

    // allocate widget structure
    Widget *widget = (Widget *)malloc(sizeof(Widget));
    widget->w = w;
    widget->parent=parent;
    widget->type=type;
    widget->text = NULL;
    widget->bmp = 0;
    memcpy(&(widget->geom),geometry,sizeof(WgGeometry));
    printf("create_window %d %d\n",(int)w,type);
    printf("x=%d y=%d\n",widget->geom.top,widget->geom.left);
    // save widget into list
    tsearch(widget,&widget_list,widget_cmp);

    return widget;
}

void wg_move(Widget *wg,int new_x, int new_y) {
    XMoveWindow(display,wg->w,new_x,new_y);
}

void wg_resize(Widget *wg,unsigned int new_width, unsigned int new_height) {

    Window w = wg->w;

    XResizeWindow(display,w,new_width,new_height);

    // find childs
    Window root,parent;
    Window *children;
    unsigned int nchildren;
    XQueryTree(display, w, &root, &parent, &children, &nchildren);

    if (nchildren==0) return;

    for(unsigned int i=0; i<nchildren; i ++) {
        Widget *child=wg_find_from_window(children[i]);
        if (!child) continue;
        int child_x,child_y;
        unsigned int child_width,child_height;
        wg_resolve_geometry(&(child->geom), wg, &child_x,&child_y, &child_width, &child_height);
        wg_resize(child,child_width,child_height);
        wg_move(child,child_x,child_y);
    }

    if (children) XFree(children);

}


void draw_widget_title_bar(Widget *wg) {

    if (wg->text == NULL ) return;

    static Bool init=False;
    static XftFont *font;
    static XRenderColor xrcolor;
    static XftColor xftcolor;
    if (!init) {

        font =  XftFontOpen (display, screen_num,
                             XFT_FAMILY, XftTypeString, "charter",
                             XFT_SIZE, XftTypeDouble, 8.0,
                             NULL);

        xrcolor.red  =21111;
        xrcolor.green=21111;
        xrcolor.blue =21111;
        xrcolor.alpha=65535;
        XftColorAllocValue(display,DefaultVisual(display,0),DefaultColormap(display,0),&xrcolor,&xftcolor);

        init = True;
    }

    // get  window size
    XWindowAttributes window_attrs;
    XGetWindowAttributes(display,wg->w,&window_attrs);

    // get text size
    XGlyphInfo extents;
    XftTextExtents8 (display,font,(XftChar8 *)wg->text, strlen(wg->text),&extents);


    XftDraw *xftdraw;
    xftdraw = XftDrawCreate(display,wg->w,DefaultVisual(display,0),DefaultColormap(display,0));

    if (extents.width>0) {
        int left = (window_attrs.width - extents.width)/2;
        XftDrawString8(xftdraw, &xftcolor, font, left, 11 , (XftChar8 *)wg->text, strlen(wg->text));

    }
    XFlush(display);

}

void draw_shadow(Window w,GC gc,int x1,int y1,int x2,int y2,XColor nw,XColor se) {

    XSetForeground(display, gc, se.pixel);
    XDrawLine(display,w,gc,x2,y2,x2,y1);
    XDrawLine(display,w,gc,x2,y2,x1,y2);

    XSetForeground(display, gc, nw.pixel);
    XDrawLine(display,w,gc,x1,y1,x2,y1);
    XDrawLine(display,w,gc,x1,y1,x1,y2);

}

void draw_widget_button(Widget *wg) {

    XImage *image,*shapeimage;
    static XpmAttributes attributes;

    char **xpm;
    switch(wg->bmp) {
    case bm_close:
        xpm = close_xpm;
        break;
    case bm_full:
        xpm = full_xpm;
        break;
    case bm_iconify:
        xpm = iconify_xpm;
        break;
    }
    XpmCreateImageFromData(display,xpm,&image,&shapeimage,&attributes);

    // draw decoration
    GC gc = XCreateGC(display, wg->w, 0, NIL);

    XPutImage(display,wg->w,gc,image,0,0,2,2,11,11);

    XDestroyImage(image);
    XDestroyImage(shapeimage);

}

void draw_widget_decoration(Widget *wg) {

    // get decoration size
    Window root_window;
    int x,y;
    unsigned int width,height,border,depth;
    XGetGeometry(display,wg->w,&root_window,&x,&y,&width,&height,&border,&depth);
    printf ("x=%d, y=%d, w=%d, h=%d,border=%d\n",x,y,width,height,border);

    // draw decoration
    GC gc = XCreateGC(display, wg->w, 0, NIL);
    draw_shadow(wg->w,gc,0,0,width-1,height-1,xcolors[col_light],xcolors[col_dark]);
    draw_shadow(wg->w,gc,DECORATION_MARGIN-1,DECORATION_MARGIN_TOP-1,width-DECORATION_MARGIN,height-DECORATION_MARGIN,xcolors[col_dark],xcolors[col_light]);

    XFlush(display);

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
    /*Window root_window;
    int x,y;
    unsigned int width,height,border,depth;
    XGetGeometry(display,window,&root_window,&x,&y,&width,&height,&border,&depth);
    printf ("x=%d, y=%d, w=%d, h=%d,border=%d\n",x,y,width,height,border);*/
    XWindowAttributes deco_attrs;
    XGetWindowAttributes(display,window,&deco_attrs);


    // create decoration
    int deco_x=deco_attrs.x-DECORATION_MARGIN;
    int deco_y=deco_attrs.y-DECORATION_MARGIN_TOP;
    int deco_w=deco_attrs.width+DECORATION_MARGIN*2;
    int deco_h=deco_attrs.height+DECORATION_MARGIN+DECORATION_MARGIN_TOP;

    if(deco_x<0) deco_x=0;
    if(deco_y<0) deco_y=0;

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

    close_button->bmp = bm_close;

    WgGeometry full_geom = { .left=-1, .top=0, .width=button_width, .height=button_width, .bottom=-1, .right=0 };
    Widget *full_button = create_widget(wg_button,title_bar,
                                        &full_geom,xcolors[col_normal]);
    full_button->bmp = bm_full;

    WgGeometry iconify_geom = { .left=-1, .top=0, .width=button_width, .height=button_width, .bottom=-1, .right=button_width-1};
    Widget *iconify_button = create_widget(wg_button,title_bar,
                                           &iconify_geom,xcolors[col_normal]);
    iconify_button->bmp = bm_iconify;

    // the x11 window itself
    WgGeometry window_geom = { .left=DECORATION_MARGIN, .top=DECORATION_MARGIN_TOP, .width=-1, .height=-1, .bottom=DECORATION_MARGIN, .right=DECORATION_MARGIN };
    wg_create_from_x(wg_x11,window,decoration,&window_geom);

    // Add to SaveSet
    XAddToSaveSet(display,window);

    // reparent window into decoration
    XReparentWindow(display,window,decoration->w,DECORATION_MARGIN,DECORATION_MARGIN_TOP);

    XFlush(display);

}

Widget *wg_find_from_window(Window w) {

    // find widget in widget list
    Widget search;
    search.w = w;

    Widget *widget = NULL;

    const void *find = tfind(&search,&widget_list,widget_cmp);
    if (!find) return NULL;

    widget = (*(Widget **)find);
    return widget;

}

void on_expose_event(XExposeEvent e) {

    // find widget in widget list
    Widget *widget = wg_find_from_window(e.window);

    if (widget==NULL) { // not found
        printf("Widget not found\n");
        return;
    }

    switch(widget->type) {
    case wg_decoration:
        draw_widget_decoration(widget);
        printf("decoration\n");

        break;

    case wg_title_bar:
        draw_widget_title_bar(widget);
        printf("title_bar\n");

        break;

    case wg_button:
        draw_widget_button(widget);
        printf("title_bar\n");

        break;


    default:

        break;
    }

}

void onclick_title_bar(XButtonPressedEvent e) {

    Window w = e.window;

    Widget *title_bar = wg_find_from_window(w);
    if (!title_bar) return;

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

void on_buttonpress_event(XButtonPressedEvent e) {

    Widget *widget = wg_find_from_window(e.window);

    if (widget==NULL) { // not found
        printf("Widget not found\n");
        return;
    }

    switch(widget->type) {

    case wg_title_bar:
        onclick_title_bar(e);
        break;
    default:
        break;

    }


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
                 SubstructureNotifyMask);


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
            printf("Expose event x=%d,y=%d,w=%d,h=%d,count=%d\n",
                   event.xexpose.x,event.xexpose.y,event.xexpose.width,event.xexpose.height,event.xexpose.count);
            on_expose_event(event.xexpose);
            break;

        case EnterNotify:
            printf("Enter event\n");
            //enter_decoration(event.xcrossing.window,event.xcrossing.x,event.xcrossing.y);
            break;

        case LeaveNotify:
            printf("Leave event\n");
            //leave_decoration(event.xcrossing.window);
            break;


        case ButtonPress:
            printf("Button Press event\n");
            on_buttonpress_event(event.xbutton);
            break;

        } // end switch


    } // end while


}
