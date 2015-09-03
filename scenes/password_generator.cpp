#include "password_generator.h"


password_generator::password_generator(CURRENT_FILE_FORMAT::st_save save)
{
    game_save = save;
    reset();
}

password_matrix password_generator::run()
{
    reset();

    if (game_save.selected_player == PLAYER_BETABOT) {
        matrix.value[1][0] = PASSWORD_BALL_COLOR_RED;
    } else if (game_save.selected_player == PLAYER_CANDYBOT) {
        matrix.value[1][0] = PASSWORD_BALL_COLOR_BLUE;
    } else if (game_save.selected_player == PLAYER_KITTYBOT) {
        matrix.value[2][1] = PASSWORD_BALL_COLOR_BLUE;
    }

    if (game_save.stages[TECHNOBOT] == 1) {
        matrix.value[5][3] = PASSWORD_BALL_COLOR_BLUE;
    }
    if (game_save.stages[DAISIEBOT] == 1) {
        matrix.value[5][5] = PASSWORD_BALL_COLOR_BLUE;
    }
    if (game_save.stages[MUMMYBOT] == 1) {
        matrix.value[3][2] = PASSWORD_BALL_COLOR_BLUE;
    }
    if (game_save.stages[SPIKEBOT] == 1) {
        matrix.value[4][5] = PASSWORD_BALL_COLOR_BLUE;
    }


    if (game_save.stages[SKULLCASTLE1] == 1) {
        matrix.value[0][0] = PASSWORD_BALL_COLOR_BLUE;
    }
    if (game_save.stages[SKULLCASTLE2] == 1) {
        matrix.value[1][1] = PASSWORD_BALL_COLOR_BLUE;
    }
    if (game_save.stages[SKULLCASTLE3] == 1) {
        matrix.value[4][0] = PASSWORD_BALL_COLOR_BLUE;
    }
    if (game_save.stages[SKULLCASTLE4] == 1) {
        matrix.value[4][2] = PASSWORD_BALL_COLOR_BLUE;
    }

    if (game_save.stages[INTRO_STAGE] == 1) {
        matrix.value[0][2] = PASSWORD_BALL_COLOR_RED;
    }
    if (game_save.stages[MAGEBOT] == 1) {
        matrix.value[2][0] = PASSWORD_BALL_COLOR_RED;
    }
    if (game_save.stages[DYNAMITEBOT] == 1) {
        matrix.value[4][1] = PASSWORD_BALL_COLOR_RED;
    }
    if (game_save.stages[SEAHORSEBOT] == 1) {
        matrix.value[2][3] = PASSWORD_BALL_COLOR_RED;
    }
    if (game_save.stages[APEBOT] == 1) {
        matrix.value[3][5] = PASSWORD_BALL_COLOR_RED;
    }

    if (game_save.items.energy_tanks == 1) {
        matrix.value[5][4] = PASSWORD_BALL_COLOR_RED;
    } else if (game_save.items.energy_tanks == 2) {
        matrix.value[3][4] = PASSWORD_BALL_COLOR_RED;
    } else if (game_save.items.energy_tanks == 3) {
        matrix.value[3][1] = PASSWORD_BALL_COLOR_RED;
    } else if (game_save.items.energy_tanks == 4) {
        matrix.value[0][4] = PASSWORD_BALL_COLOR_RED;
    } else if (game_save.items.energy_tanks == 5) {
        matrix.value[4][3] = PASSWORD_BALL_COLOR_RED;
    } else if (game_save.items.energy_tanks == 6) {
        matrix.value[1][3] = PASSWORD_BALL_COLOR_RED;
    } else if (game_save.items.energy_tanks == 7) {
        matrix.value[2][2] = PASSWORD_BALL_COLOR_RED;
    } else if (game_save.items.energy_tanks == 8) {
        matrix.value[1][5] = PASSWORD_BALL_COLOR_RED;
    } else if (game_save.items.energy_tanks == 9) {
        matrix.value[5][0] = PASSWORD_BALL_COLOR_RED;
    }

    // weapon-tank: y 5, x 0
    if (game_save.items.weapon_tanks == 1) {
        matrix.value[5][0] = PASSWORD_BALL_COLOR_RED;
    }

    // special tank y 5, x 2
    if (game_save.items.special_tanks == 1) {
        matrix.value[5][2] = PASSWORD_BALL_COLOR_RED;
    }
    // usados, linha 4: 5, 3, 0
    // linha4, coluna 1 -> armor, 2 -> hands, 4 -> legs
    if (game_save.armor_pieces[ARMOR_BODY]) {
        matrix.value[1][4] = PASSWORD_BALL_COLOR_RED;
    }
    if (game_save.armor_pieces[ARMOR_ARMS]) {
        matrix.value[2][4] = PASSWORD_BALL_COLOR_RED;
    }
    if (game_save.armor_pieces[ARMOR_LEGS]) {
        matrix.value[4][4] = PASSWORD_BALL_COLOR_RED;
    }

    return matrix;

}

void password_generator::reset()
{
    for (int i=0; i<PASSWORD_GRID_SIZE; i++) {
        for (int j=0; j<PASSWORD_GRID_SIZE; j++) {
            matrix.value[i][j] = 0;
        }
    }
}
