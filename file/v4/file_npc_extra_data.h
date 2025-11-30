#ifndef FILE_NPC_EXTRA_DATA_H
#define FILE_NPC_EXTRA_DATA_H

typedef signed char Sint8;

namespace format_v4 {
    struct file_npc_extra_data {
        Sint8 id;                   // serial number
        float jump_accel_x = 0;     // jump acceleration in the X vertice
        float jump_accel_y = 0;     // jump acceleration in the Y vertice
        float jump_speed_x = 1;     // jump initial speed in the X vertice
        float jump_speed_y = 1;     // jump initial speed in the Y vertice

        // TBD //
        int extra_data_001 = 0;
        int extra_data_002 = 0;
        int extra_data_003 = 0;
        int extra_data_004 = 0;
        int extra_data_005 = 0;
        int extra_data_006 = 0;

        float extra_data_007 = 0;
        float extra_data_008 = 0;
        float extra_data_009 = 0;
        float extra_data_010 = 0;
        float extra_data_011 = 0;
        float extra_data_012 = 0;
    };
}

#endif // FILE_NPC_EXTRA_DATA_H
