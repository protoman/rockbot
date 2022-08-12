#ifndef FILE_SAVE_301_H
#define FILE_SAVE_301_H

#include <sys/types.h>
#include <sys/stat.h>

#include "defines.h"

typedef signed char Sint8;

namespace format_v4 {

    struct st_items {
        int energy_tanks;
        int weapon_tanks;
        int special_tanks;
        int balancer;
        int lifes;
        int bolts;
        bool heart_pieces[WEAPON_COUNT];                        // one heart-piece for each stage
        int weapons[WEAPON_COUNT];

        // items that can be bought on shop
        bool exit;                                              // leave a completed stage
        bool shock_guard;                                       // do not move back when hit
        bool half_damage;                                       // takes only half the damage
        bool spike_guard;                                       // take gerular damage on spikes instead of dying
        bool speed_up;                                          // faster movement
        bool hyper_jump;                                        // bigger jump
        bool energy_saver;                                      // weapons consume half the wpn-energy
        bool speed_shot;                                        // normal shots become faster (can shoot more often)
        bool power_shot;                                        // charged shots causes more damage

        st_items() {
            energy_tanks = 0;
            weapon_tanks = 0;
            special_tanks = 0;
            balancer = 0;
            lifes = 3;
            bolts = 0;
            for (int i=0; i<WEAPON_COUNT; i++) {
                weapons[i] = PLAYER_INITIAL_HP;
                heart_pieces[i] = false;
            }
            // shop items
            exit = false;
            shock_guard = false;
            half_damage = false;
            speed_up = false;
            hyper_jump = false;
            energy_saver = false;
            speed_shot = false;
            power_shot = false;
            spike_guard = false;
        }
    };


    struct st_items_v1 {
        unsigned short int energy_tanks; /**< TODO */ /**< TODO */
        unsigned short int weapon_tanks; /**< TODO */
        unsigned short int special_tanks; /**< TODO */
        unsigned short int balancer; /**< TODO */
        unsigned short int lifes; /**< TODO */
        unsigned short int bolts; /**< TODO */
        unsigned short int weapons[WEAPON_COUNT]; /**< TODO */
        // items that can be bought on shop
        bool exit;
        unsigned short int shock_guard; // do not move back when hit. consumed after finished/game-over the stage
        unsigned short int half_damage; // takes only half the damage (if > 1). consumed after finished/game-over the stage
        unsigned short int spike_guard;
        bool speed_up;                  // faster movement
        bool hyper_jump;                // bigger jump
        bool energy_saver;              // weapons consume half the wpn-energy
        bool speed_shot;                // normal shots become faster (can shoot more often)
        bool power_shot;                // charged shots causes more damage

        st_items_v1() {
            energy_tanks = 0;
            weapon_tanks = 0;
            special_tanks = 0;
            balancer = 0;
            lifes = 3;
            bolts = 0;
            for (int i=0; i<WEAPON_COUNT; i++) {
                weapons[i] = PLAYER_INITIAL_HP;
            }
            // shop items
            exit = false;
            shock_guard = 0;
            half_damage = 0;
            speed_up = false;
            hyper_jump = false;
            energy_saver = false;
            speed_shot = false;
            power_shot = false;
            spike_guard = 0;
        }
    };


    // this stores any "generic" items you could want to add to game, such as parts od beta to fix him (like in mmx zero)
    // or anything we want to expand later, like shop items
    struct st_collect_items {
        int id;
        int number;
        st_collect_items() {
            id = 0;
            number = 0;
        }
    };


    struct st_save_v1 {
        Sint8 finished_stages;                                         // number of already finished stages, just to simplify calc
        Sint8 stages[MAX_STAGES];                                      // 0 still open, 1 finished
        st_items items;
        Sint8 selected_player;
        Sint8 difficulty;                                              // 0 - easy, 1 - normal, 2 - hard
        Sint8 defeated_enemies_count;                                  // number of defeated enemies
        bool used_countinue;
        // NEW VALUES V3
        bool armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX_V1];

        st_save_v1() {
            finished_stages = 0;
            for (int i=0; i<MAX_STAGES; i++) {
                stages[i] = 0;
            }
            selected_player = 0;
            difficulty = 1;
            defeated_enemies_count = 0;
            used_countinue = false;
            for (int i=0; i<FS_PLAYER_ARMOR_PIECES_MAX_V1; i++) {
                armor_pieces[i] = false;
            }
        }
    };


    struct st_save {
        int finished_stages;                                         // number of already finished stages, just to simplify calc
        int stages[MAX_STAGES];                                      // 0 still open, 1 finished
        st_items items;
        int selected_player;
        int difficulty;                                              // 0 - easy, 1 - normal, 2 - hard
        bool used_countinue;
        bool armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX];
        st_collect_items collect_items[FS_COMMON_ITEMS_MAX];
        int xp_points;
        int level;
        int defeated_enemies_count;                                  // number of defeated enemies
        int defeated_bosses_count;                                   // number of defeated bosses
        int continues_used;
        bool damaged_boss_with_only_normal;
        int used_e_tanks;
        int used_w_tanks;

        st_save() {
            reset();
        }

        void reset() {
            finished_stages = 0;
            for (int i=0; i<MAX_STAGES; i++) {
                stages[i] = 0;
            }
            selected_player = 0;
            difficulty = DIFFICULTY_NORMAL;
            defeated_enemies_count = 0;
            used_countinue = false;
            for (int i=0; i<FS_PLAYER_ARMOR_PIECES_MAX; i++) {
                armor_pieces[i] = false;
            }
            xp_points = 0;
            level = 0;
            defeated_bosses_count = 0;
            continues_used = 0;
            damaged_boss_with_only_normal = true;
            used_e_tanks = 0;
            used_w_tanks = 0;
        }
    };

}

#endif // FILE_SAVE_H
