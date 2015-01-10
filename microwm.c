
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
	for (int i=0;i<col_count;i++) {
		XParseColor(display,colormap,colors_text[i],&xcolors[i]);
		XAllocColor(display, colormap, &xcolors[i]);
	}

}

int widget_cmp(const void *wg1,const void *wg2) {
	Widget *wwg1=(Widget *)wg1;
	Widget *wwg2=(Widget *)wg2;
	return wwg1->w - wwg2->w;
}

Widget *create_widget(widget_type type,Window parent,int x,int y,unsigned int width,unsigned int height,XColor color) {

	// create window
	Window w = XCreateSimpleWindow(display, parent,x,y,width,height, 0, NIL, color.pixel);

	// allocate widget structure
	Widget *widget = malloc(sizeof(widget));
	widget->w = w;
	widget->type=type;
printf("create_window %d %d\n",w,type);
	// save widget into list
	tsearch(widget,&widget_list,widget_cmp);	

	// add override_redirect to the decoration
	XSetWindowAttributes attributes;
	attributes.override_redirect = True;
	XChangeWindowAttributes(display,w,CWOverrideRedirect,&attributes);

	// add event 
	XSelectInput(display, w , ExposureMask | EnterWindowMask | LeaveWindowMask);

	// map window
 	XMapWindow(display, w);


	/*switch(type) {
		case wg_button:
			widget->on_expose = &draw_widget_button;
			break;

		case wg_title_bar:
			widget->on_expose = &draw_widget_title_bar;
			break;

		case wg_decoration:
			widget->on_expose = &draw_widget_decoration;
			break;
		
		default:
			widget->on_expose = NULL;
			break;
	}*/

	return widget;
}


void draw_widget_title_bar(Widget *wg) {

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
	Window root_window;
	int x,y;
	unsigned int width,height,border,depth;
	XGetGeometry(display,wg->w,&root_window,&x,&y,&width,&height,&border,&depth);

	// get text size
	XGlyphInfo extents;
	XftTextExtents8 (display,font,(XftChar8 *)wg->text, strlen(wg->text),&extents);
      

	XftDraw *xftdraw;
	xftdraw = XftDrawCreate(display,wg->w,DefaultVisual(display,0),DefaultColormap(display,0));

	if (extents.width>0) {
		int left = (width - extents.width)/2;
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
	XpmAttributes attributes;
	printf("idi");
	XpmCreateImageFromData(display,wg->bitmap,&image,&shapeimage,&attributes);
	printf("aa");
	// get decoration size
	Window root_window;
	int x,y;
	unsigned int width,height,border,depth;
	XGetGeometry(display,wg->w,&root_window,&x,&y,&width,&height,&border,&depth);

	// draw decoration
	GC gc = XCreateGC(display, wg->w, 0, NIL);
	//draw_shadow(wg->w,gc,0,0,width-1,height-1,xcolors[col_light],xcolors[col_dark]);

	XPutImage(display,wg->w,gc,image,0,0,2,2,11,11);

}

void draw_widget_decoration(Widget *wg) {

	/*static Bool init=False;
	static XColor light,dark;

	if (!init) {
		XParseColor(display,colormap,"#e9e9e9",&light);
		XAllocColor(display, colormap, &light);
		XParseColor(display,colormap,"#7b7b7b",&dark);
		XAllocColor(display, colormap, &dark);
		init = True;
	}*/

	// get decoration size
	Window root_window;
	int x,y;
	unsigned int width,height,border,depth;
	XGetGeometry(display,wg->w,&root_window,&x,&y,&width,&height,&border,&depth);
	printf ("x=%d, y=%d, w=%d, h=%d,border=%d\n",x,y,width,height,border);

	// draw decoration
	GC gc = XCreateGC(display, wg->w, 0, NIL);
	draw_shadow(wg->w,gc,0,0,width-1,height-1,xcolors[col_light],xcolors[col_dark]);
	draw_shadow(wg->w,gc,deco_l-1,deco_t-1,width-deco_l,height-deco_b,xcolors[col_dark],xcolors[col_light]);

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
	Window root_window;
	int x,y;
	unsigned int width,height,border,depth;
	XGetGeometry(display,window,&root_window,&x,&y,&width,&height,&border,&depth);
	printf ("x=%d, y=%d, w=%d, h=%d,border=%d\n",x,y,width,height,border);

	// create decoration
	int deco_x=x-deco_l;
	int deco_y=y-deco_t;
	int deco_w=width+deco_l+deco_r;
	int deco_h=height+deco_t+deco_b;

	if(deco_x<0) deco_x=0;
	if(deco_y<0) deco_y=0;

	// decoration frame
	Widget *decoration = create_widget(wg_decoration, DefaultRootWindow(display), 
		deco_x,deco_y,deco_w,deco_h,xcolors[col_normal]);

	// add title bar
	int title_width=deco_w-deco_l-deco_r;
	int title_height=deco_t-deco_b;
	Widget *title_bar = create_widget(wg_title_bar, decoration->w, 
		deco_l,deco_b-1,title_width,title_height,xcolors[col_normal]);

	// get window title
	get_window_name(window,&(title_bar->text));
	printf("name=%s\n",title_bar->text);

	// add buttons
	int button_width=title_height-1;
	Widget *close_button = create_widget(wg_button,title_bar->w,
		0,0,button_width,button_width,xcolors[col_normal]);
	close_button->bitmap = close_xpm;

	Widget *full_button = create_widget(wg_button,title_bar->w,
		title_width-button_width,0,button_width,button_width,xcolors[col_normal]);
	full_button->bitmap = full_xpm;

	Widget *iconify_button = create_widget(wg_button,title_bar->w,
		title_width-button_width*2,0,button_width,button_width,xcolors[col_normal]);
	iconify_button->bitmap = iconify_xpm;

	// reparent window into decoration
	XReparentWindow(display,window,decoration->w,deco_l,deco_t);

	XFlush(display);

}


void on_expose_event(XExposeEvent e) {

/*draw_window_decoration(e.window);
return;*/

	// find widget in widget list
	Widget search;
	search.w = e.window;

	Widget *widget = NULL;
	
	const void *find = tfind(&search,&widget_list,widget_cmp);
	widget = (*(Widget **)find);

	if (widget==NULL) { // not found
		printf("Widget not found\n");
		return;
	}

	/*if (!widget->on_expose == NULL) {
		widget->on_expose(widget);
	}*/

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
				//draw_window_decoration(event.xexpose.window);
			break;

			case EnterNotify:
				printf("Enter event\n");
				//enter_decoration(event.xcrossing.window,event.xcrossing.x,event.xcrossing.y);
				break;

			case LeaveNotify:
				printf("Leave event\n");
				//leave_decoration(event.xcrossing.window);
				break;

		} // end switch


	} // end while


}
