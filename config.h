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

#ifndef config_h
#define config_h

#define CONFIG_FILE "~/.microwm" ///< the config file name

#define DECORATION_MARGIN (_config[cfg_frame_margin].number)  ///< The distance between the border of the X window and the decoration

#define DECORATION_MARGIN_TOP (_config[cfg_frame_margin_top].number)   ///< The title bar height

#define MIN_WINDOW_WIDTH (2*DECORATION_MARGIN+16)   ///< minimum width of a X window
#define MIN_WINDOW_HEIGHT (DECORATION_MARGIN+DECORATION_MARGIN_TOP+16) ///< minimum height of a X window

enum {
    cfg_col_light,
    cfg_col_dark,
    cfg_col_normal,
    cfg_col_title,
    cfg_col_title_focus,
    cfg_frame_margin,
    cfg_frame_margin_top,
    cfg_title_bar_font_name,
    cfg_title_bar_font_size,
    cfg_title_bar_font_color_unfocus,
    cfg_title_bar_font_color_focus,
    cfg_title_bar_font_weight,

    cfg_count
};

typedef struct {
    char *name;
    union {
    char *string;
    int number;
    };
} ConfigElement;

#endif // config_h
