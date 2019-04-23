#ifndef CASTLEPOINTS_H
#define CASTLEPOINTS_H

#include "defines.h"
#include "file/format/st_common.h"

namespace format_v4 {
    struct st_file_castle_ponts {
        st_position points[CASTLE_STAGES_MAX];
    };
}


#endif // CASTLEPOINTS_H
