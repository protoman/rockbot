#ifndef FILE_STRINGS_H
#define FILE_STRINGS_H

#include <string>

#include "defines.h"
#include "file/format/st_common.h"

#define SCENE_TEXT_LINES_N 6
#define SCENE_OBJECTS_N 20

namespace format_v4 {

    struct st_common_string {
        std::string name;
        std::string value;
        st_common_string() {
            name = std::string("");
            value = std::string("");
        }
        st_common_string(std::string set_name, std::string set_value) {
            name = set_name;
            value = set_value;
        }
    };

} // namespace

#endif // FILE_STRINGS_H
