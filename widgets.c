#include <stdio.h>
#include <stdlib.h>
#include <search.h>

#include "widgets.h"

// widgets tree
void *widget_list = NULL;

// the X11 display must be declared somewhere
extern Display *display;
extern int screen_num;

extern Cursor xcursors[];
extern XColor xcolors[];

// widgets functions
// ******************



// compare function for LIBC trees
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



Widget *wg_create(widget_type type,Widget *parent,WgGeometry *geometry,XColor color) {

    int x,y;
    unsigned int width,height;

    wg_resolve_geometry(geometry,parent,&x,&y,&width,&height);

    // create window
    Window parent_window;
    if (parent==NULL)
        parent_window=DefaultRootWindow(display);
    else
        parent_window=parent->w;

    Window w = XCreateSimpleWindow(display, parent_window,x,y,width,height, 0, NIL, color.pixel);

    // add override_redirect
	XSetWindowAttributes attributes = { .override_redirect = True };
	XChangeWindowAttributes(display,w,CWOverrideRedirect,&attributes);

    Widget *new_widget =  wg_create_from_x(type,w,parent,geometry);

    // add event
    long event_mask = 0;
    switch(type) {
        case wg_decoration:
            event_mask =  ExposureMask | ButtonPressMask | ButtonReleaseMask |PointerMotionMask | SubstructureRedirectMask | SubstructureNotifyMask;
            new_widget->on_expose = &draw_widget_decoration;
            break;
        case wg_title_bar:
            event_mask =  ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
            new_widget->on_expose = &draw_widget_title_bar;
            break;
        case wg_button:
            event_mask =  ExposureMask | ButtonPressMask;
            new_widget->on_expose = &draw_widget_button;
            break;
        default:
            event_mask =  ExposureMask | ButtonPressMask ;

    }

    XSelectInput(display, new_widget->w , event_mask );

    XDefineCursor(display, new_widget->w, xcursors[0]);

    // map window
    //XMapWindow(display, new_widget->w);

    return new_widget;
}

// recursively destroy a widget and is childs
void wg_destroy(Widget *widget) {

    printf("wg_destroy %d\n",widget->type);

    // find childs
    Window root,parent;
    Window *children;
    unsigned int nchildren;
    XQueryTree(display, widget->w, &root, &parent, &children, &nchildren);

    for(unsigned int i=0; i<nchildren; i ++) {
        Widget *child=wg_find_from_window(children[i]);
        if (!child) continue;
        wg_destroy(child);
    }

printf("Destroy widget %d\n",widget->type);
         // first destroy the x window
        if (widget->type != wg_x11) {
                XUnmapWindow(display,widget->w);
                XDestroyWindow(display,widget->w);
        }

        // then the widget
        if (widget->text) free(widget->text);
        if (widget->wm_window) free (widget->wm_window);

        tdelete(widget,&widget_list,widget_cmp);
        free(widget);


    if (children) XFree(children);


}

Widget *wg_create_from_x(widget_type type,Window w,Widget *parent,WgGeometry *geometry) {

    // allocate widget structure
    Widget *widget = (Widget *)malloc(sizeof(Widget));
    widget->w = w;
    widget->parent=parent;
    widget->type=type;
    widget->text = NULL;
    widget->xpm = NULL;
	widget->wm_window = NULL;
    widget->on_click = NULL;
    widget->on_motion = NULL;
    widget->on_unmap = NULL;
    widget->on_expose = NULL;
    memcpy(&(widget->geom),geometry,sizeof(WgGeometry));
    printf("create_window %d %d\n",(int)w,type);

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

// Drawing functions
// ******************

void draw_widget_title_bar(Widget *wg,XExposeEvent e) {

    if (wg->text == NULL ) return;

    static Bool init=False;
    static XftFont *font;
    static XRenderColor rcolor_fg,rcolor_bg;
    static XftColor fcolor_fg,fcolor_bg;
    if (!init) {

        font =  XftFontOpen (display, screen_num,
                             XFT_FAMILY, XftTypeString, "charter",
                             XFT_SIZE, XftTypeDouble, 8.0,
                             NULL);

        rcolor_fg.red  = xcolors[col_title_focus].red;
        rcolor_fg.green=xcolors[col_title_focus].green;
        rcolor_fg.blue =xcolors[col_title_focus].blue;
        rcolor_fg.alpha=65535;
        XftColorAllocValue(display,DefaultVisual(display,0),DefaultColormap(display,0),&rcolor_fg,&fcolor_fg);

        rcolor_bg.red  = xcolors[col_normal].red;
        rcolor_bg.green=xcolors[col_normal].green;
        rcolor_bg.blue =xcolors[col_normal].blue;
        rcolor_bg.alpha=65535;
        XftColorAllocValue(display,DefaultVisual(display,0),DefaultColormap(display,0),&rcolor_bg,&fcolor_bg);

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
        XftDrawRect(xftdraw, &fcolor_bg,left,0,extents.width,11);
        XftDrawString8(xftdraw, &fcolor_fg, font, left, 11 , (XftChar8 *)wg->text, strlen(wg->text));

    }

    XftDrawDestroy(xftdraw);
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

void draw_widget_button(Widget *wg,XExposeEvent e) {

    XImage *image,*shapeimage;
    static XpmAttributes attributes;

    XpmCreateImageFromData(display,wg->xpm,&image,&shapeimage,&attributes);

    // draw decoration
    GC gc = XCreateGC(display, wg->w, 0, NIL);

    // clipping
    //XRectangle rectangle =  { .x=e.x, .y=e.y, .width = e.width, .height = e.height } ;
    //XSetClipRectangles(display,gc,0,0,&rectangle,1,Unsorted);

    XPutImage(display,wg->w,gc,image,0,0,2,2,11,11);

    XDestroyImage(image);
    XDestroyImage(shapeimage);
    XFreeGC(display,gc);

}

void draw_widget_decoration(Widget *wg,XExposeEvent e) {

    // get decoration size
    Window root_window;
    int x,y;
    unsigned int width,height,border,depth;
    XGetGeometry(display,wg->w,&root_window,&x,&y,&width,&height,&border,&depth);
    //printf ("x=%d, y=%d, w=%d, h=%d,border=%d\n",x,y,width,height,border);

    // draw decoration
    GC gc = XCreateGC(display, wg->w, 0, NIL);

    // clipping
    //XRectangle rectangle =  { .x=e.x, .y=e.y, .width = e.width, .height = e.height } ;
    //XSetClipRectangles(display,gc,0,0,&rectangle,1,Unsorted);


    draw_shadow(wg->w,gc,0,0,width-1,height-1,xcolors[col_light],xcolors[col_dark]);
    draw_shadow(wg->w,gc,DECORATION_MARGIN-1,DECORATION_MARGIN_TOP-1,width-DECORATION_MARGIN,height-DECORATION_MARGIN,xcolors[col_dark],xcolors[col_light]);

    XFlush(display);
    XFreeGC(display,gc);

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

void wg_destroy_all() {

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
        Widget *wg = wg_find_from_window(*child);
        if (!wg) continue;
        wg_destroy(wg);
    }

    if (children) XFree(children);

    // ungrap display
    XUngrabServer(display);

}
