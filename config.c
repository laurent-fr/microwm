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
#include <stdlib.h> // atoi
#include <ctype.h>  // isalnum, isdigit
#include <string.h> // strdup

#include <unistd.h> // find home directory
#include <sys/types.h>
#include <pwd.h>

#include "config.h"

/// The default values for the config file
ConfigElement _config[cfg_count] = {
    [cfg_col_dark] = { .name ="color.dark", .type=type_string, .string = "#978d8d" },
    [cfg_col_normal] = { .name ="color.normal", .type=type_string, .string = "#dbdbdb" },
    [cfg_col_light] = { .name ="color.light", .type=type_string, .string = "#f4f4f4" },
    [cfg_frame_margin] = { .name ="frame.margin", .type=type_number, .number=4 },
    [cfg_frame_margin_top] = { .name ="frame.margin_top", .type=type_number, .number=20 },
    [cfg_title_bar_font_name] = { .name ="title_bar.font.name", .type=type_string, .string = "charter" },
    [cfg_title_bar_font_size] = { .name ="title_bar.font.size", .type=type_number, .number = 8 },
    [cfg_title_bar_font_color_unfocus] = { .name ="title_bar.color.unfocus", .type=type_string, .string = "#333333" },
    [cfg_title_bar_font_color_focus] = { .name ="title_bar.color.focus", .type=type_string, .string = "#d06610" },
    [cfg_title_bar_font_weight] = { .name ="title_bar.font.weight", .type=type_string, .string="bold" }

};

/// \brief read the config file
///
/// the file location is ~/.microwm
/// use defaults if the file doesn't exist
///
void config_load() {

    char line[MAX_LINE_LENGTH];
    char filename[MAX_FILENAME_LENGTH];

    // find home directory of the user
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;

    // build config file name
    snprintf(filename,MAX_FILENAME_LENGTH,"%s/%s",homedir,CONFIG_FILE);

    // read the config file
    FILE *fp=fopen(filename,"r");
    if (fp==NULL) {
        fprintf(stderr,"Cannot read configuration file %s.\n",CONFIG_FILE);
        return;
    }

    int line_number=0;
    while(fgets(line,MAX_LINE_LENGTH,fp)!=NULL) {
            line_number ++;

            // parse the current line
            char key[MAX_KEY_LENGTH];
            char value[MAX_VALUE_LENGTH];
            int success = config_parse_line(line,key,value);

            // parse error
            // TODO : write the error in plain text
            if (success<parse_ok) {
                printf("Error reading %s, line %d: %d\n",filename,line_number,success);
                continue;
            }

            // empty line
            if (success==parse_ok_empty) continue;

            // find the key
            int key_num = config_find_key(key);

            // key not found
            if (key_num<0) {
                fprintf(stderr,"Error reading %s, line %d: Unknown key '%s'.\n",filename,line_number,key);
                continue;
            }

            // the value is a string
            if (success==parse_ok_found_string) {
                if (_config[key_num].type!=type_string) {
                    fprintf(stderr,"Error reading %s, line %d: Value should be a String for key %s.",
                            filename,line_number,key);
                    continue;
                }
                printf("key=%s , value=(STRING)%s\n",key,value);
                _config[key_num].string=strdup(value);
            }

            // the value is a number
            if (success==parse_ok_found_number) {
                if (_config[key_num].type!=type_number) {
                    fprintf(stderr,"Error reading %s, line %d: Value should be a Number for key %s.",
                            filename,line_number,key);
                    continue;
                }
                printf("key=%s , value=(NUMBER)%s\n",key,value);
               _config[key_num].number=atoi(value);
            }
    }

    fclose(fp);

}

/// \brief find the numeric config key from the equivalent string
///
/// \param key the key string
/// \return the key number if found, -1 if not found
///
int config_find_key(char *key) {

    for(int i=0;i<cfg_count;i++) { if (!strcmp(_config[i].name,key)) return i; }

    return -1;
}

/// \brief parse a config line
///
/// \param line the line to parse
/// \param key (output) the key
/// \param value (output) the value
/// \return parse error or success
///
/// * If there's a parse error, the return value is **negative** ( parse_error_... )
/// * If the line is empty, a **zero** value is returned ( parse_ok_empty )
/// * If a key and value are found, a **positive** value is returned ( parse_ok_found_... )
///
int config_parse_line(char *line,char key[], char value[]) {

    // a valid line is either :
    //   mode_start mode_key mode_equal mode_value_number
    // or
    //   mode_start mode_key mode_equal mode_value_string mode_string_end
    // or
    //   mode_start
    enum { mode_start, mode_key, mode_equal, mode_value_number, mode_value_string , mode_string_end };

    int mode=mode_start;

    int key_index=0;
    int value_index=0;

    key[0]='\0'; value[0]='\0';

    char *charptr=line;

    while(*charptr) {
        char charcur=*charptr;
        charptr ++;

        // \n
        if (charcur=='\n')  break;

        // #
        if (charcur=='#') {
            if (mode==mode_value_string) { value[value_index++]=charcur; continue; }  // # in a string
            break; // start of a comment
        }

        // space tab
        if ( (charcur==' ') || (charcur=='\t') ) {
            if (mode==mode_value_string) value[value_index++]=charcur; // in a string
            continue; // ignore
        }

        // "
        if (charcur=='"') {
            if (mode==mode_value_string) { mode=mode_string_end; continue; }
            if (mode==mode_equal) { mode=mode_value_string; continue ; }
            return parse_error_bad_quote;
        }

        // =
        if (charcur=='=')   {
            if (mode==mode_key) { mode=mode_equal; continue; }
            if (mode==mode_value_string) { value[value_index++]=charcur; continue; }
            return parse_error_equal;
        }

        // digit
        if (isdigit(charcur)) {
            if (mode==mode_equal) mode=mode_value_number;
            if ( (mode==mode_value_number) || (mode==mode_value_string) ) { value[value_index++]=charcur; continue; }
            return parse_error_bad_digit;
        }

        // alpha . _
        if ( isalpha(charcur) || (charcur=='.') || (charcur=='_') ) {
            if (mode==mode_start) mode=mode_key;
            if (mode==mode_key) { key[key_index++]=charcur; continue; }
            if (mode==mode_value_string) { value[value_index++]=charcur; continue; }
            return parse_error_bad_data;
        }

        // TODO : test key_index and value_index

    }

    if (mode==mode_start) return parse_ok_empty;

    if (key_index==0) return parse_error_no_key;
    if (value_index==0) return parse_error_no_value;
    if ((mode!=mode_string_end) && (mode!=mode_value_number)) return parse_error_unterminated_line;

    key[key_index]='\0';
    value[value_index]='\0';

   // printf("%s=%s\n",key,value);

    if (mode==mode_string_end) return parse_ok_found_string;
    if (mode==mode_value_number) return parse_ok_found_number;

    return parse_ok; // for the compiler ;  we never go here ...
}
