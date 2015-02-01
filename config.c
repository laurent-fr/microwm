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

#include "config.h"


ConfigElement _config[cfg_count] = {
    [cfg_col_dark] = { .name ="color.dark", .string = "#978d8d" },
    [cfg_col_normal] = { .name ="color.normal", .string = "#dbdbdb" },
    [cfg_col_light] = { .name ="color.light", .string = "#f4f4f4" },
    [cfg_frame_margin] = { .name ="frame.margin", .number=4 },
    [cfg_frame_margin_top] = { .name ="frame.margin_top", .number=20 },
    [cfg_title_bar_font_name] = { .name ="title_bar.font.name", .string = "charter" },
    [cfg_title_bar_font_size] = { .name ="title_bar.font.size", .number = 8 },
    [cfg_title_bar_font_color_unfocus] = { .name ="title_bar.color.unfocus", .string = "#333333" },
    [cfg_title_bar_font_color_focus] = { .name ="title_bar.color.focus", .string = "#d06610" },
    [cfg_title_bar_font_weight] = { .name ="title_bar.font.weight", .string="bold" }

};


void config_load() {

}
