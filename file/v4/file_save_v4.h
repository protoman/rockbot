#ifndef FILE_SAVE_301_H
#define FILE_SAVE_301_H

namespace format_v4 {

    struct st_items {
        Uint8 energy_tanks;
        Uint8 weapon_tanks;
        Uint8 special_tanks;
        Uint8 balancer;
        Uint8 lifes;
        Uint8 bolts;
        Uint8 weapons[WEAPON_COUNT];

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


    // this stores any "generic" items you could want to add to game, such as parts od beta to fix him (like in mmx zero)
    // or anything we want to expand later, like shop items
    struct st_collect_items {
        Uint8 id;
        Uint8 number;
        collect_items() {
            id = 0;
            number = 0;
        }
    };


    struct st_save {
        Sint8 finished_stages;                                         // number of already finished stages, just to simplify calc
        Sint8 stages[MAX_STAGES];                                      // 0 still open, 1 finished
        st_items items;
        Sint8 selected_player;
        Sint8 difficulty;                                              // 0 - easy, 1 - normal, 2 - hard
        Sint8 defeated_enemies_count;                                  // number of defeated enemies
        bool used_countinue;
        bool armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX];
        st_collect_items collect_items[FS_COMMON_ITEMS_MAX];

        st_save() {
            finished_stages = 0;
            for (int i=0; i<MAX_STAGES; i++) {
                stages[i] = 0;
            }
            selected_player = 0;
            difficulty = 1;
            defeated_enemies_count = 0;
            used_countinue = false;
            for (int i=0; i<FS_PLAYER_ARMOR_PIECES_MAX; i++) {
                armor_pieces[i] = false;
            }
        }
    };

}

#endif // FILE_SAVE_H
