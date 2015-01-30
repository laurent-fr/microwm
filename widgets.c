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
#include <search.h>

#include "widgets.h"

// widgets tree
void *widget_list = NULL;  ///< the widget tree (libc tree)

// the X11 display must be declared somewhere
extern Display *display;
extern int screen_num;

extern Cursor xcursors[];
extern XColor xcolors[];
extern XftFont *_xft_font;

// widgets functions
// ******************



/// \brief compare function for LIBC trees
///
/// \param wg1 first widget
/// \param wg2 second widget
/// \return difference of widget ids
///
/// this compare function is mandatory for libc trees
///
int widget_cmp(const void *wg1,const void *wg2) {
    Widget *wwg1=(Widget *)wg1;
    Widget *wwg2=(Widget *)wg2;
    return wwg1->w - wwg2->w;
}

/// \brief resolve the geometry constraints for a widget
///
/// \param geom pointer to a WgGeometry structure
/// \param parent the widget containing the widget for which we are calculating the constraints
/// \param x (output) the x position of the widget (from is parent)
/// \param y (output) the y position of the widget (from is parent)
/// \param width (output) the width of the widget
/// \param height (output) the height of the widget
///
/// positive or null values are mandatory contraints, negative values are to be calculated.
///
/// * if left,top,width and height are given, the position of the widget is fixed
/// * if width is negative, left and right are needed (variable width widget)
/// * if height is negative, top and bottom are needed (variable height widget)
/// * left and width / top and height : left or top fixed position
/// * right and width / bottom and width : right or bottom fixed position
///
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


/// \brief create a widget
///
/// \param type the type of the widget
/// \param parent the parent widget, or NULL if the parent is the desktop
/// \param geometry the geometry constraints
/// \param color the background color of the widget
///
/// The function also :
/// * set default X events mask according to the type of the widget
/// * set a default cursor
///
/// \note the X window needs to be mapped with XMapWindow
/// \note malloc() a new Widget structure, must be freed with wg_destroy()
///
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

    return new_widget;
}

/// \brief recursively destroy a widget and its childs
///
/// \param widget the first widget to destroy
///
/// * the widget structure is freed
/// * the window is unmapped **unless** it's a x11 type
///
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

/// \brief create a widget embedding a given X window
///
/// \param type the widget type
/// \param w the X Window
/// \param parent the parent widget, or NULL if the parent is the desktop
/// \param geometry the geometry constraints
///
/// \note malloc() a new Widget structure, must be freed with wg_destroy()
///
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

/// \brief move a widget
///
/// \param wg the widget
/// \param new_x new x position (from parent widget point of view)
/// \param new_y new y position (from parent widget point of view)
///
void wg_move(Widget *wg,int new_x, int new_y) {
    XMoveWindow(display,wg->w,new_x,new_y);
}

/// \brief resize a widget
///
/// \param wg the widget
/// \param new_width new width
/// \param new_height new height
///
/// Childs widgets are also resized according to geometry constraints
///
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

/// \brief paint a title bar
///
/// \param wg the widget
/// \param e XExposeEvent
///
/// The text is centered on a background rectangle
///
void draw_widget_title_bar(Widget *wg,XExposeEvent e) {

    if (wg->text == NULL ) return;

    static Bool init=False;
    static XftColor fcolors[col_count];

    if (!init) {

        for (int i=0;i<col_count;i ++) {
            XRenderColor rcolor;
            rcolor.red  = xcolors[i].red;
            rcolor.green=xcolors[i].green;
            rcolor.blue =xcolors[i].blue;
            rcolor.alpha=65535;
            XftColorAllocValue(display,DefaultVisual(display,0),DefaultColormap(display,0),&rcolor,&fcolors[i]);
        }

        init = True;
    }

    // get  window size
    XWindowAttributes window_attrs;
    XGetWindowAttributes(display,wg->w,&window_attrs);

    // get text size
    XGlyphInfo extents;
    XftTextExtents8 (display, _xft_font,(XftChar8 *)wg->text, strlen(wg->text),&extents);


    XftDraw *xftdraw;
    xftdraw = XftDrawCreate(display,wg->w,DefaultVisual(display,0),DefaultColormap(display,0));

    if (extents.width>0) {

        // clipping
        Region region = XCreateRegion();
        XRectangle rect = { .x= e.x, .y = e.y, .width = e.width , .height = e.height };
        XUnionRectWithRegion(&rect,region,region);  // union of an empty region with a rectangle
                                                    // output = the same region, which now as the size of the rectangle
        XftDrawSetClip(xftdraw,region);
        XDestroyRegion(region);

        // draw the title
        int left = (window_attrs.width - extents.width)/2;
        XftDrawRect(xftdraw, &fcolors[col_normal],left,0,extents.width,11);
        XftDrawString8(xftdraw, &fcolors[wg->fg_color], _xft_font, left, 11 , (XftChar8 *)wg->text, strlen(wg->text));

    }

    XftDrawDestroy(xftdraw);
    XFlush(display);

}

/// \brief draw a shadow on the border of a X Window
///
/// \param w the X Window
/// \param gc the graphic context
/// \param x1 first corner x
/// \param y1 first corner y
/// \param x2 second corner x
/// \param y2 second corner y
/// \param nw color of the north-west lines
/// \param se color of the south-east lines
///
/// * light nw/dark se color for volume effect
/// * dark nw/light se color for depth effect
///
void draw_shadow(Window w,GC gc,int x1,int y1,int x2,int y2,XColor nw,XColor se) {

    XSetForeground(display, gc, se.pixel);
    XDrawLine(display,w,gc,x2,y2,x2,y1);
    XDrawLine(display,w,gc,x2,y2,x1,y2);

    XSetForeground(display, gc, nw.pixel);
    XDrawLine(display,w,gc,x1,y1,x2,y1);
    XDrawLine(display,w,gc,x1,y1,x1,y2);

}

/// \brief paint a button
///
/// \param wg the button
/// \param e the XExposeEvent
///
/// Draw a pixmap in the center of the button
///
void draw_widget_button(Widget *wg,XExposeEvent e) {

    XImage *image,*shapeimage;
    static XpmAttributes attributes;

    XpmCreateImageFromData(display,wg->xpm,&image,&shapeimage,&attributes);

    // draw decoration
    GC gc = XCreateGC(display, wg->w, 0, NIL);

    // clipping
    XRectangle rectangle =  { .x=0, .y=0, .width = e.width, .height = e.height } ;
    XSetClipRectangles(display,gc,e.x,e.y,&rectangle,1,Unsorted);

    XPutImage(display,wg->w,gc,image,0,0,2,2,11,11);

    XDestroyImage(image);
    XDestroyImage(shapeimage);
    XFreeGC(display,gc);

}
/// \brief paint a window decoration
///
/// \param wg the decoration widget
/// \param e the XExposeEvent
///
/// Draw a shadow on the border of the window
///
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
    XRectangle rectangle =  { .x=0, .y=0, .width = e.width, .height = e.height } ;
    XSetClipRectangles(display,gc,e.x,e.y,&rectangle,1,Unsorted);


    draw_shadow(wg->w,gc,0,0,width-1,height-1,xcolors[col_light],xcolors[col_dark]);
    draw_shadow(wg->w,gc,DECORATION_MARGIN-1,DECORATION_MARGIN_TOP-1,width-DECORATION_MARGIN,height-DECORATION_MARGIN,xcolors[col_dark],xcolors[col_light]);

    XFlush(display);
    XFreeGC(display,gc);

}

/// \brief find the widget embedding a X Window
///
/// \param w the X Window
/// \return a widget if found, NULL otherwise
///
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

/// \brief destroy all the widgets
///
/// \bug XQueryTree hangs ...
///
void wg_destroy_all() {

    //find root windows
    Window root_return,parent;
    Window *children,*child;
    unsigned int nchildren=0,i;
    Window root = RootWindow(display, screen_num);

    XQueryTree(display, root, &root_return, &parent, &children, &nchildren);

    for(i=0, child=children; i<nchildren; i ++,child++) {
        Widget *wg = wg_find_from_window(*child);
        if (!wg) continue;
        wg_destroy(wg);
    }

    if (children) XFree(children);

}
