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

#ifndef icccm_h
#define icccm_h

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

typedef struct {

    int min_width;
    int min_height;
    int max_width;
    int max_height;
    int width_inc;
    int height_inc;

} icccm_size_hints;

Status get_window_name(Window w, char **name);
Bool has_wm_delete_window(Window w);
void get_wm_normal_hints(Window w,icccm_size_hints *hints);

#endif // icccm_h
