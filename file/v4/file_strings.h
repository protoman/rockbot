#ifndef FILE_STRINGS_H
#define FILE_STRINGS_H

#include <string>

#include "defines.h"
#include "file/format/st_common.h"

#define SCENE_TEXT_LINES_N 6
#define SCENE_OBJECTS_N 20

namespace format_v4 {

        struct st_file_common_string {
            char name[FS_COMMONSTRING_ID_SIZE];
            char value[FS_COMMON_STRINGS_DIALOG];
            st_file_common_string() {
                name[0] = '\0';
                value[0] = '\0';
            }
            st_file_common_string(std::string new_name, std::string new_value) {
                set_name(new_name);
                set_value(new_value);
            }
            void set_name(std::string new_name) {
                if (new_name.length() >= FS_COMMONSTRING_ID_SIZE) {
                    new_name = new_name.substr(0, FS_COMMONSTRING_ID_SIZE-1);
                }
                sprintf(name, "%s", new_name.c_str());
            }
            void set_value(std::string new_value) {
                if (new_value.length() >= FS_COMMON_STRINGS_DIALOG) {
                    new_value = new_value.substr(0, FS_COMMON_STRINGS_DIALOG-1);
                }
                sprintf(value, "%s", new_value.c_str());
            }
    };

} // namespace

#endif // FILE_STRINGS_H
