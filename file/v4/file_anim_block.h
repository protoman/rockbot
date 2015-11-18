#ifndef FILE_ANIM_BLOCK_H
#define FILE_ANIM_BLOCK_H

#include "defines.h"

namespace format_v4 {
    struct file_map_anim_block {
        int x;
        int y;
        file_map_anim_block() {
            x = 0;
            y = 0;
        }
        file_map_anim_block(int set_x, int set_y) {
            x = set_x;
            y = set_y;
        }
    };

    struct file_anim_block {
        char filename[FS_CHAR_FILENAME_SIZE];
        int frame_delay[FS_ANIM_TILE_MAX_FRAMES];
        file_anim_block() {
            filename[0] = '\0';
            for (int i=0; i<FS_ANIM_TILE_MAX_FRAMES; i++) {
                frame_delay[i] = 200;
            }
        }
    };
}

#endif // FILE_ANIM_BLOCK_H

