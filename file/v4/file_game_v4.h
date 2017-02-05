#ifndef V_4_H
#define V_4_H

#include <stdio.h>
#include <iostream>
#include "defines.h"
#include "file/format/st_common.h"
#include "file/format/st_hitPoints.h"

#include "file/v4/file_stage_v4.h"
#include "file/v4/file_save_v4.h"
#include "file/v4/file_config_v4.h"
#include "file/v4/file_scene_v4.h"


// @NOTE: all defines related to filesystem must contain a "FS_" prefix


//#define FORCE_CONVERT 1

namespace format_v4 {

    struct file_projectile {
        char name[CHAR_NAME_SIZE];
        char graphic_filename[FS_CHAR_NAME_SIZE];
        st_size size;
        bool is_destructible;                                   // indicates if the projectile can be shot down, if true, uses HP
        Uint8 hp;                                               // how much danage needs to be taken to destruct the projectile
        Uint8 trajectory;                                       // RENAMED from projectile_type in 2.1
        Uint8 max_shots;                                        // number of maximum simultaneous projectiles, if 0, is infinite
        Uint8 speed;                                            // now many pixels are moved into a step
        Uint8 damage;                                           // how many hitpoints will be reduced from target
        // new in 3.0.1
        char sfx_filename[FS_CHAR_NAME_SIZE];                  // allows to use different projectile shot sound
        bool can_be_reflected;                                  // if false, like in a charged-shot, can't be reflected by shield
        Uint8 spawn_npc_id;                                     // will spawn NPCs (like bird egg in mm2)
        Uint8 spawn_npc_n;                                      // number of NPCs that will be spawned
        file_projectile() {
            sprintf(name, "%s", "Projectile");
            sprintf(graphic_filename, "%s", "projectile_normal.png");
            is_destructible = false;
            hp = 1;
            size.width = 6;
            size.height = 6;
            trajectory = TRAJECTORY_LINEAR;
            max_shots = 3;
            speed = PROJECTILE_DEFAULT_SPEED;
            damage = PROJECTILE_DEFAULT_DAMAGE;
            sfx_filename[0] = '\0';
            can_be_reflected = true;
            spawn_npc_id = -1;
            spawn_npc_n = 0;
        }
    };

    struct file_projectilev2 {
        char name[CHAR_NAME_SIZE];
        char graphic_filename[FS_CHAR_NAME_SIZE];
        st_size size;
        bool is_destructible;                                   // indicates if the projectile can be shot down, if true, uses HP
        Uint8 hp;                                               // how much danage needs to be taken to destruct the projectile
        Uint8 trajectory;                                       // RENAMED from projectile_type in 2.1
        Uint8 max_shots;                                        // number of maximum simultaneous projectiles, if 0, is infinite
        Uint8 speed;                                            // now many pixels are moved into a step
        Uint8 damage;                                           // how many hitpoints will be reduced from target
        // new in 3.0.1
        char sfx_filename[FS_CHAR_NAME_SIZE];                  // allows to use different projectile shot sound
        bool can_be_reflected;                                  // if false, like in a charged-shot, can't be reflected by shield
        Uint8 spawn_npc_id;                                     // will spawn NPCs (like bird egg in mm2)
        Uint8 spawn_npc_n;
        bool is_explosive;
        // number of NPCs that will be spawned
        file_projectilev2() {
            sprintf(name, "%s", "Projectile");
            sprintf(graphic_filename, "%s", "projectile_normal.png");
            is_destructible = false;
            hp = 1;
            size.width = 6;
            size.height = 6;
            trajectory = TRAJECTORY_LINEAR;
            max_shots = 3;
            speed = PROJECTILE_DEFAULT_SPEED;
            damage = PROJECTILE_DEFAULT_DAMAGE;
            sfx_filename[0] = '\0';
            can_be_reflected = true;
            spawn_npc_id = -1;
            spawn_npc_n = 0;
            is_explosive = false;
        }

        file_projectilev2(file_projectile v1) {
            sprintf(name, "%s", v1.name);
            sprintf(graphic_filename, "%s", v1.graphic_filename);
            size.width = v1.size.width;
            size.height = v1.size.height;
            is_destructible = v1.is_destructible;
            hp = v1.hp;
            trajectory = v1.trajectory;
            max_shots = v1.max_shots;
            speed = v1.speed;
            damage = v1.damage;
            sprintf(sfx_filename, "%s", v1.sfx_filename);
            can_be_reflected = v1.can_be_reflected;
            spawn_npc_id = v1.spawn_npc_id;
            spawn_npc_n = v1.spawn_npc_n;
            is_explosive = false;
        }
    };


    struct st_sprite_data {
        bool used;                                                  // if true, the sprite exists
        Uint16 duration;
        Uint8 sprite_graphic_pos_x;
        st_rectangle collision_rect;
        st_sprite_data() {
            used = false;
            duration = 0;
            sprite_graphic_pos_x = 0;
        }

        st_sprite_data & operator=(st_sprite_data new_value) {
            used = new_value.used;
            duration = new_value.duration;
            sprite_graphic_pos_x = new_value.sprite_graphic_pos_x;
            collision_rect = new_value.collision_rect;
            return *this;
        }
    };


    struct file_weapon_colors { // DONE -> Player
        st_color color1;
        st_color color2;
        st_color color3;
    };

    struct file_player { // DONE -> Game
        char name[FS_CHAR_NAME_SIZE];
        char graphic_filename[FS_CHAR_NAME_SIZE];
        char face_filename[FS_CHAR_NAME_SIZE];
        Uint8 HP;
        st_size_int8 sprite_size;                                       // size of sprite graphic
        st_rectangle sprite_hit_area;                                   // the area of the graphic where is used for hit/collision
        Uint8 move_speed;                                               // how many sprites move each step
        st_sprite_data sprites[ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT];
        file_weapon_colors weapon_colors[MAX_WEAPON_N];
        // habilities part
        bool have_shield;
        Uint8 max_shots;                                                // number of maximum simultaneous projectiles
        Uint8 simultaneous_shots;                                       // number of projectiles shot at one button press
        bool can_double_jump;
        bool can_slide;                                                 // if false, dashes instead of sliding
        bool can_charge_shot;
        Sint8 full_charged_projectile_id;
        /// NEW IN FILE-FORMAT 3.0
        bool can_air_dash;
        Sint8 damage_modifier;
        bool can_shot_diagonal;
        st_position_int8 attack_arm_pos;
        Uint8 attack_frame;
        bool double_shot;

        file_player(int n) {
            sprintf(name, "PLAYER[%d]", n);
            sprintf(graphic_filename, "%s%d%s", "p", (n+1), ".png");
            face_filename[0] = '\0';
            HP = 0;
            sprite_size.width = 29;
            sprite_size.height = 29;
            sprite_hit_area.x = 0;
            sprite_hit_area.y = 0;
            sprite_hit_area.w = 29;
            sprite_hit_area.h = 29;
            move_speed = 2.0;
            max_shots = 3;
            simultaneous_shots = 1;
            can_double_jump = false;
            have_shield = false;
            can_slide = false;
            can_charge_shot = false;
            full_charged_projectile_id = -1;
            can_air_dash = false;
            damage_modifier = 0;
            can_shot_diagonal = false;
            attack_frame = 0;

            /// === DEFAULT VALUES === //
            // ROCK
            if (n == 0) {
                weapon_colors[0].color1 = st_color(143, 0, 119);
                weapon_colors[0].color2 = st_color(0, 115, 239);
                have_shield = true;
                can_slide = true;
                can_charge_shot = true;
            } else if (n == 1) {
                // CANDY
                weapon_colors[0].color1 = st_color(191, 0, 191);
                weapon_colors[0].color2 = st_color(131, 0, 243);
                weapon_colors[0].color3 = st_color(166, 80, 239);
                can_double_jump = true;
                can_shot_diagonal = true;
                damage_modifier = 1;
            } else if (n == 2) {
                // BETA
                weapon_colors[0].color1 = st_color(255, 51, 0);
                weapon_colors[0].color2 = st_color(255, 155, 59);
                weapon_colors[0].color3 = st_color(230, 255, 0);
                max_shots = 4;
                damage_modifier = -1;
                simultaneous_shots = 2;
            } else {
                // KITTY
                weapon_colors[0].color1 = st_color(255, 51, 0);
                weapon_colors[0].color2 = st_color(255, 155, 59);
                weapon_colors[0].color3 = st_color(230, 255, 0);
                have_shield = true;
                can_charge_shot = true;
                can_air_dash = true;
            }
        }

        file_player() {
            file_player(0);
        }
    };



    struct file_weapon { // DONE - Game
        Uint8 id_projectile;
        char name[CHAR_NAME_SIZE];
        Uint8 damage;
        Uint8 id_projectile_charged;
        file_weapon() {
            sprintf(name, "%s", "Weapon");
            damage = 1;
            id_projectile = 0;
            id_projectile_charged = 0;
        }
    };

    struct file_object { // DONE - Game
        char name[CHAR_NAME_SIZE];
        char graphic_filename[FS_CHAR_NAME_SIZE];
        Sint8 type;													///@ TODO: use enum
        int timer;													// used as time for disapearing block
        Sint8 speed;												// used as speed for moving platform
        Sint16 limit;												// used as range (pixels) for moving platform, visible time for disapearing block
        Sint8 direction;											// used to check if moving away from oiginalpoint or moving to it
        Sint16 distance;
        st_size_int8 size;
        Sint16 frame_duration;										// indicates what is the used frame
        bool animation_auto_start;                                  // false means it will not be animated until started
        bool animation_reverse;                                     // start animating
        bool animation_loop;                                        // true = animation will loop, false = animation will run once
        file_object() {
            sprintf(name, "%s", "Object");
            graphic_filename[0] = '\0';
            animation_auto_start = true;
            animation_reverse = false;
            animation_loop = true;
            frame_duration = 100;
            type = -1;
            timer = 0;
            speed = 2;
            limit = 0;
            animation_auto_start = true;
            animation_reverse = false;
            animation_loop = true;
        }
    };


    struct st_weakness {
        Sint8 weapon_id;                                           // weapon number of weakness, if -1, no weakness
        Sint8 damage_multiplier;                                   // how many time more damage is taken, if zero, no damage from this weapon is taken
        st_weakness() {
            weapon_id = -1;
            damage_multiplier = 1;
        }
    };


    struct file_npc { // DONE - Game
        Sint8 id;                                                  // serial number
        //unsigned int projectile_id;                              // indicates the projectile ID (relation with file_projectile)
        Sint8 projectile_id[2];                                    // indicate the id of an attack the NCP can user
        char name[CHAR_NAME_SIZE];
        char graphic_filename[FS_CHAR_NAME_SIZE];
        struct st_hit_points hp;
        Sint8 direction;                                           // defines how it behavies (kink of AI*)
        Sint8 speed;                                               // defines the distances it can see enemies
        Sint16 walk_range;                                         // defines how long from the start point it can go
        Sint8 facing;                                              // defines the side npc is facing before start moving (also used by LINEWALK behavior)
        struct st_position start_point;
        st_sprite_data sprites[ANIM_TYPE_COUNT][ANIM_FRAMES_COUNT];// changed in 2.0.4
        st_size_int8 frame_size;
        bool is_ghost;
        Sint8 shield_type;
        Sint8 IA_type;                                             // IA types. For custom (edited) ones, use IA_TYPES_COUNT + position (in the array)
        Sint8 fly_flag;                                            // 0 - can't fly, 1 - flyer (...)
        st_weakness weakness[FS_NPC_WEAKNESSES];                                    // each npc have weaknesses for each boss weapon, plus the normal weapon
        char bg_graphic_filename[FS_CHAR_NAME_SIZE];               // holds a static background
        st_position sprites_pos_bg;                                // holds position of sprites in relation with background
        bool is_boss;                                              // indicates if this NPC is a boss
        bool is_sub_boss;                                          // a middle-stage boss, the doors will only open after it's dead
        int respawn_delay;                                         // if > 0, will respawn even if on-screen
        st_position_int8 attack_arm_pos;
        Uint8 attack_frame;


    /**
     * @brief
     *
     */
        file_npc() {
            id = -1;
            projectile_id[0] = -1;
            projectile_id[1] = -1;
            sprintf(name, "%s", "Enemy Name");
            graphic_filename[0] = '\0';
            direction = ANIM_DIRECTION_LEFT;
            speed = 3;
            walk_range = 160;
            facing = ANIM_DIRECTION_LEFT;
            frame_size.width = TILESIZE;
            frame_size.height = TILESIZE;
            is_ghost = false;
            shield_type = 0;
            IA_type = 0;
            fly_flag = 0;
            bg_graphic_filename[0] = '\0';
            is_boss = false;
            is_sub_boss = false;
            respawn_delay = 0;
            attack_frame = -1;
        }

    };





    struct file_ai_action {
        int chance;
        int action;
        int go_to;                                                  // action number to be executed once this one is finished, -1 indicates that must return to CHANCES (random)
        int go_to_delay;                                            // delay before going to the next action
        int extra_parameter;										// left, right, etc

    /**
     * @brief
     *
     */
        file_ai_action()
        {
            chance = 0;
            action = 0;
            go_to = 0;
            go_to_delay = 500;
            extra_parameter = 0;
        }
    };

    struct reaction {
        int action;
        int go_to;                                                  // action number to be executed once this one is finished, -1 indicates that must return to CHANCES (random)
        int go_to_delay;                                            // delay before going to the next action
        int extra_parameter;										// left, right, etc
        reaction() {
            action = -1;
            go_to = 0;
            go_to_delay = 500;
            extra_parameter = 0;
        }
    };

    struct file_artificial_inteligence {
        char name[CHAR_NAME_SIZE];
        struct file_ai_action states[AI_MAX_STATES];
        struct reaction reactions[MAX_AI_REACTIONS];                    // reactions near, hit and dead

        file_artificial_inteligence() {
            sprintf(name, "%s", "A.I.");
        }
    };







    struct st_checkpoint {
        int map; /**< TODO */
        int x; /**< TODO */
        int y; /**< TODO */
        int map_scroll_x; /**< TODO */
                st_checkpoint() {
                    reset();
                }
                void reset() {
                    map = 0;
                    x = RES_W/2-29/2;
                    y = -1;
                    map_scroll_x = 0;
                }
        };

    // CAMPOS NOVOS OU NÃO-USADOS //
    struct file_intro {
        int initial_delay;                                  // delay before showing text and effects
        char bg_filename[FS_CHAR_FILENAME_SIZE];            // background image
        int transition_effect;                              // how we transition to next screen
        // text (3 lines x2)
    };

    struct st_shop_dialog {
        char face_graphics_filename[20];
        char line1[FS_DIALOG_LINES][DIALOG_LINE_LIMIT];
    };


    enum e_trophy_conditions {
        TROPHY_CONDITIONS_NO_DAMAGE,                                    // finish a stage without being hit
        TROPHY_CONDITIONS_END_GAME,                                     // finish the game
        TROPHY_CONDITIONS_KILLER,                                       // kill 1000 enemies @TODO - add counter to save
        TROPHY_CONDITIONS_NO_CONTINUES,                                 // finish the game without using any continue @TODO - add flag in save
        TROPHY_CONDITIONS_COUNT
    };

    struct st_file_trophy {
        Uint8 condition;
        char name[FS_NAME_SIZE];
        char filename[FS_FILENAME_SIZE];
    };


    struct st_armor_piece {
        int special_ability[FS_MAX_PLAYERS]; // each player can have a different ability
        int got_message[FS_MAX_PLAYERS][FS_DIALOG_LINES];

        st_armor_piece() {
            for (int i=0; i<FS_MAX_PLAYERS; i++) {
                special_ability[i] = 0;
                for (int j=0; j<FS_DIALOG_LINES; j++) {
                    got_message[i][j] = -1;
                }
            }
        }
    };


    struct file_game {

        // *** WARNINGN *** //
        // any changes made here must be reflected into file_io::write_game() and file_io::read_game() //

        float version;
        char name[FS_CHAR_NAME_SIZE];
        //file_projectile projectiles[FS_MAX_PROJECTILES];
        file_weapon weapons[FS_MAX_WEAPONS];                            // 8 stage-bosses and 3 item-like
        Sint8 semi_charged_projectile_id;                               // common to all players
        Sint8 player_items[FS_PLATER_ITEMS_N];                          // common to all players -> to be used in add_coil_object and add_jet_object
        char stage_face_filename[MAX_STAGES][FS_FACE_FILENAME_MAX];
        char stages_face_name[MAX_STAGES][FS_CHAR8_NAME_SIZE];           // name of the boss to be shown in the stage-select screen

        st_file_trophy trophies[TROPHIES_MAX];
        st_armor_piece armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX];

        char boss_music_filename[FS_CHAR_NAME_SIZE];
        char final_boss_music_filename[FS_CHAR_NAME_SIZE];
        char got_weapon_music_filename[FS_CHAR_NAME_SIZE];
        char game_over_music_filename[FS_CHAR_NAME_SIZE];
        char stage_select_music_filename[FS_CHAR_NAME_SIZE];
        char game_start_screen_music_filename[FS_CHAR_NAME_SIZE];
        bool use_second_castle;
        Uint8 game_style;                                               // if 1, stages are executed one after another, like castlevania or ghouls & ghosts
        Uint8 final_boss_id;                                            // indicates who is the final boss, the id refers to the map_npc id, not to the npc itself


        st_color weapon_menu_colors[MAX_WEAPON_N];

        // CONSTRUCTOR //
        file_game() {
            /// *** hardcoded parts *** ///
            version = 4.00; // file-format version, not game
            sprintf(name, "%s", "My Game");
            semi_charged_projectile_id = 0;
            player_items[0] = 0;
            player_items[1] = 0;

            // ** NEW IN FILE V4.0 ** //
            boss_music_filename[0] = '\0';
            final_boss_music_filename[0] = '\0';
            got_weapon_music_filename[0] = '\0';
            game_over_music_filename[0] = '\0';
            stage_select_music_filename[0] = '\0';
            game_start_screen_music_filename[0] = '\0';
            game_style = 0;
            use_second_castle = false;
            final_boss_id = -1;

            for (int i=0; i<MAX_STAGES; i++) {
                stage_face_filename[i][0] = '\0';
            }

            for (int i=0; i<FS_MAX_WEAPONS; i++) {
                sprintf(weapons[i].name, "Weapon [%d]", i);
            }
        }



    };





    // ******************************************* STAGES ********************************************************************** //



} // NAMESPACE




#endif // V_4_H
