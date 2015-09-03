#ifndef PASSWORD_GENERATOR_H
#define PASSWORD_GENERATOR_H

#include "../defines.h"
#include "../file/version.h"
#include "../file/file_io.h"


enum PASSWORD_BALL_COLORS {
    PASSWORD_BALL_COLOR_NOCOLOR, PASSWORD_BALL_COLOR_RED, PASSWORD_BALL_COLOR_BLUE
};

struct password_matrix {
    int value[PASSWORD_GRID_SIZE][PASSWORD_GRID_SIZE];

    password_matrix() {
        for (int i=0; i<PASSWORD_GRID_SIZE; i++) {
            for (int j=0; j<PASSWORD_GRID_SIZE; j++) {
                value[i][j] = 0;
            }
        }
    }
};

class password_generator
{
public:
    password_generator(CURRENT_FILE_FORMAT::st_save save);

     struct password_matrix run();

private:
     struct password_matrix matrix;
     CURRENT_FILE_FORMAT::st_save game_save;
     void reset();
};

#endif // PASSWORD_GENERATOR_H
