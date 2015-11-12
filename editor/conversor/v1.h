#ifndef V1_H
#define V1_H

#include <SDL/SDL.h>


#define V1_FS_COLORCYCLE_MAX_ITEMS 10
#define V1_FS_CHAR_FILENAME_SIZE 30
#define V1_MAP_W 200
#define V1_MAP_H 15
#define V1_FS_MAX_MAP_NPCS 30
#define V1_FS_MAX_MAP_OBJECTS 30
#define V1_FS_DIALOG_LINES 3
#define V1_DIALOG_LINE_LIMIT 30
#define V1_FS_MAX_PLAYERS 4
#define V1_CASTLE_STAGES_MAX 4
#define V1_FS_FILENAME_SIZE 30
#define V1_FS_CHAR_NAME_SIZE 30
#define V1_FS_STAGE_MAX_MAPS 3
#define V1_FS_STAGE_MAX_LINKS 30
#define V1_FS_MAX_STAGES 20
#define V1_FS_CHAR_NAME_SIZE 30
#define V1_CHAR_NAME_SIZE 30
#define V1_CHAR_FILENAME_SIZE 30
#define V1_FS_GAME_MAX_NPCS 100
#define V1_FS_MAX_PROJECTILES 30
#define V1_FS_MAX_WEAPONS 20
#define V1_FS_MAX_AI_TYPES 50
#define V1_FS_ANIM_TILES_MAX 20
#define V1_GAME_MAX_OBJS 60
#define V1_FS_GAME_MAX_OBJS 40
#define V1_PROJECTILE_DEFAULT_SPEED 10
#define V1_PROJECTILE_DEFAULT_DAMAGE 1
#define V1_PROJECTILE_DEFAULT_ANIMATION_TIME 100
#define V1_ANIM_FRAMES_COUNT 10
#define V1_MAX_WEAPON_N 12
#define V1_FS_NPC_WEAKNESSES 9
#define V1_AI_MAX_STATES 8
#define V1_MAX_AI_REACTIONS 3
#define V1_FS_NAME_SIZE 30
#define V1_FS_PLATER_ITEMS_N 2
#define V1_MAX_STAGES 16
#define V1_FS_FACE_FILENAME_MAX 20
#define V1_TROPHIES_MAX 10

enum V1_TERRAIN_TYPES {
    V1_TERRAIN_UNBLOCKED,
    V1_TERRAIN_SOLID,
    V1_TERRAIN_STAIR,
    V1_TERRAIN_DOOR,
    V1_TERRAIN_SPIKE,
    V1_TERRAIN_WATER,
    V1_TERRAIN_ICE,
    V1_TERRAIN_MOVE_LEFT,
    V1_TERRAIN_MOVE_RIGHT,
    V1_TERRAIN_SAND,
    V1_TERRAIN_SCROLL_LOCK,
    V1_TERRAIN_CHECKPOINT,
    V1_TERRAIN_EASYMODEBLOCK,
    V1_TERRAIN_HARDCODEBLOCK,
    V1_TERRAIN_TYPES_COUNT
};

enum V1_LINK_TYPES {
    V1_LINK_HORIZONTAL,
    V1_LINK_VERTICAL,
    V1_LINK_TELEPORTER,
    V1_LINK_BOSS_TELEPORTER,
    V1_LINK_FINAL_BOSS_ROOM,
    V1_LINK_TYPE_COUNT
};


enum V1_COLOR_KEYS {
    V1_COLOR_KEY_GREEN,
    V1_COLOR_KEY_PURPLE,
    V1_COLOR_KEY_CYAN,
    V1_COLOR_KEY_LENGTH
};


enum V1_e_ARMOR_PIECES {
    V1_ARMOR_ARMS,
    V1_ARMOR_BODY,
    V1_ARMOR_LEGS,
    V1_FS_PLAYER_ARMOR_PIECES_MAX
};

enum V1_PROJECTILE_TRAJECTORIES {
    V1_TRAJECTORY_LINEAR,
    V1_TRAJECTORY_ARC,
    V1_TRAJECTORY_SIN,
    V1_TRAJECTORY_CHAIN,
    V1_TRAJECTORY_QUAKE,
    V1_TRAJECTORY_BOMB,
    V1_TRAJECTORY_FOLLOW,
    V1_TRAJECTORY_PROGRESSIVE,
    V1_TRAJECTORY_FREEZE,
    V1_TRAJECTORY_DIAGONAL_UP,
    V1_TRAJECTORY_DIAGONAL_DOWN,
    V1_TRAJECTORY_CENTERED,                // centered around character
    V1_TRAJECTORY_ZIGZAG,                  // linear until reach a wall, then return (repeat until reflection number = 3)
    V1_TRAJECTORY_TARGET_DIRECTION,        // adjust to linear, diagonal up or diagonal down depending on player position when shoot
    V1_TRAJECTORY_ARC_TO_TARGET,           // forms an arn that will end at player's position
    V1_TRAJECTORY_TARGET_EXACT,            // will go exactly to the point the target is in
    V1_TRAJECTORY_FALL_BOMB,               // falls until ground then explodes
    V1_TRAJECTORY_LASER,
    V1_PROJECTILE_TRAJECTORIES_COUNT
};


enum V1_ANIM_TYPE {
    V1_ANIM_TYPE_STAND,
    V1_ANIM_TYPE_WALK,
    V1_ANIM_TYPE_JUMP,
    V1_ANIM_TYPE_ATTACK,
    V1_ANIM_TYPE_TELEPORT,
    V1_ANIM_TYPE_JUMP_ATTACK,
    V1_ANIM_TYPE_WALK_ATTACK,
    V1_ANIM_TYPE_HIT,
    V1_ANIM_TYPE_STAIRS,
    V1_ANIM_TYPE_STAIRS_SEMI,
    V1_ANIM_TYPE_STAIRS_ATTACK,
    V1_ANIM_TYPE_STAIRS_MOVE,
    V1_ANIM_TYPE_THROW,
    V1_ANIM_TYPE_TURN,
    V1_ANIM_TYPE_MOVE_SEMI, // like turn, the start of the movement
    V1_ANIM_TYPE_WALK_AIR, // for those that can walk on air
    V1_ANIM_TYPE_INTRO, // for bosses
    V1_ANIM_TYPE_SPECIAL_ATTACK, // for bosses, charging weapon, etc
    V1_ANIM_TYPE_SLIDE,
    V1_ANIM_TYPE_SHIELD,
    V1_ANIM_TYPE_VERTICAL_TURN,
    V1_ANIM_TYPE_ATTACK_THROW,
    V1_ANIM_TYPE_ATTACK_DIAGONAL_UP,
    V1_ANIM_TYPE_ATTACK_DIAGONAL_DOWN,
    V1_ANIM_TYPE_ATTACK_UP,
    V1_ANIM_TYPE_ATTACK_DOWN,
    V1_ANIM_TYPE_ATTACK_SPECIAL, // (hadouken, shouryuken, etc)
    V1_ANIM_TYPE_LOOK_UP,
    V1_ANIM_TYPE_GRAB_WALL,
    V1_ANIM_TYPE_COUNT
};

struct v1_st_position {
    short x;
    short y;
    v1_st_position() {
        x = 0;
        y = 0;
    }
    v1_st_position (int setX, int setY) {
        x = setX;
        y = setY;
    }
    v1_st_position& operator=(const v1_st_position &set_pt)
    {
        x = set_pt.x;
        y = set_pt.y;
        return *this;
    }
    bool operator==(const v1_st_position &comp_pt) const
    {
        if (x == comp_pt.x && y == comp_pt.y) {
            return true;
        }
        return false;
    }
};


struct v1_st_float_position {
    float x;
    float y;
    v1_st_float_position() {
        x = 1;
        y = 1;
    }
    v1_st_float_position (float setX, float setY) {
        x = setX;
        y = setY;
    }
    v1_st_float_position& operator=(const v1_st_position &set_pt)
    {
        x = set_pt.x;
        y = set_pt.y;
        return *this;
    }
    bool operator==(const v1_st_position &comp_pt) const
    {
        if (x == comp_pt.x && y == comp_pt.y) {
            return true;
        }
        return false;
    }
    v1_st_float_position& operator=(const v1_st_float_position &set_pt)
    {
        x = set_pt.x;
        y = set_pt.y;
        return *this;
    }
    bool operator==(const v1_st_float_position &comp_pt) const
    {
        if (x == comp_pt.x && y == comp_pt.y) {
            return true;
        }
        return false;
    }
};

struct v1_st_position_int8 {
    Sint8 x;
    Sint8 y;
    v1_st_position_int8() {
        x = 0;
        y = 0;
    }
    v1_st_position_int8 (int setX, int setY) {
        x = setX;
        y = setY;
    }
    v1_st_position_int8& operator=(const v1_st_position_int8 &set_pt)
    {
        x = set_pt.x;
        y = set_pt.y;
        return *this;
    }
    bool operator==(const v1_st_position_int8 &comp_pt) const
    {
        if (x == comp_pt.x && y == comp_pt.y) {
            return true;
        }
        return false;
    }
    bool operator!=(const v1_st_position_int8 &comp_pt) const
    {
        if (x != comp_pt.x || y != comp_pt.y) {
            return true;
        }
        return false;
    }
};


struct v1_st_position_uint8 {
    Uint8 x;
    Uint8 y;
    v1_st_position_uint8() {
        x = 0;
        y = 0;
    }
    v1_st_position_uint8 (int setX, int setY) {
        x = setX;
        y = setY;
    }
    v1_st_position_uint8& operator=(const v1_st_position_uint8 &set_pt)
    {
        x = set_pt.x;
        y = set_pt.y;
        return *this;
    }
    bool operator==(const v1_st_position_uint8 &comp_pt) const
    {
        if (x == comp_pt.x && y == comp_pt.y) {
            return true;
        }
        return false;
    }
};


struct v1_st_size_int8 {
    Sint8 width;
    Sint8 height;
    v1_st_size_int8()
    {
        width = 0;
        height = 0;
    }
    v1_st_size_int8(int w, int h)
    {
        width = w;
        height = h;
    }
};

struct v1_st_size {
    short width;
    short height;
    v1_st_size()
    {
        width = 0;
        height = 0;
    }
    v1_st_size(int w, int h)
    {
        width = w;
        height = h;
    }
};

struct v1_st_rectangle {
    short x;
    short y;
    short w;
    short h;
    v1_st_rectangle () {
        x = 0;
        y = 0;
        w = 0;
        h = 0;
    }

    v1_st_rectangle (int setX, int setY, int setW, int setH) {
        x = setX;
        y = setY;
        w = setW;
        h = setH;
    }
};

struct v1_st_color {
    short r;
    short g;
    short b;
    v1_st_color () {
        r = -1;
        g = -1;
        b = -1;
    }
    v1_st_color (int setR, int setG, int setB) {
        r = setR;
        g = setG;
        b = setB;
    }
};


struct v1_st_anim_map_tile {
    int delay;
    char filename[V1_FS_CHAR_FILENAME_SIZE];
    v1_st_anim_map_tile() {
        delay = 100;
        filename[0] = '\0';
    }
};


struct v1_st_hit_points {
    short int total; /**< TODO */
    short int current; /**< TODO */
    v1_st_hit_points (short int set_total, short int set_current)
    {
        total = set_total;
        current = set_current;
    }
    v1_st_hit_points()
    {
        total = 1;
        current = 1;
    }
};


struct v1_file_projectile {
    char name[V1_CHAR_NAME_SIZE];
    char graphic_filename[V1_CHAR_FILENAME_SIZE];
    v1_st_size_int8 size;
    bool is_destructible;                                   // indicates if the projectile can be shot down, if true, uses HP
    Uint8 hp;                                               // how much danage needs to be taken to destruct the projectile
    enum V1_PROJECTILE_TRAJECTORIES trajectory;                // RENAMED from projectile_type in 2.1
    Uint8 max_shots;                                        // number of maximum simultaneous projectiles, if 0, is infinite
    Uint8 speed;                                            // now many pixels are moved into a step
    Uint8 damage;                                           // how many hitpoints will be reduced from target
    // new in 3.0.1
    char sfx_filename[V1_CHAR_FILENAME_SIZE];                  // allows to use different projectile shot sound
    bool can_be_reflected;                                  // if false, like in a charged-shot, can't be reflected by shield
    Uint8 spawn_npc_id;                                     // will spawn NPCs (like bird egg in mm2)
    Uint8 spawn_npc_n;                                      // number of NPCs that will be spawned
    v1_file_projectile() {
        sprintf(name, "%s", "Projectile");
        graphic_filename[0] = '\0';
        is_destructible = false;
        hp = 1;
        size.width = 3;
        size.height = 2;
        trajectory = V1_TRAJECTORY_LINEAR;
        max_shots = 3;
        speed = V1_PROJECTILE_DEFAULT_SPEED;
        damage = V1_PROJECTILE_DEFAULT_DAMAGE;
        sfx_filename[0] = '\0';
        can_be_reflected = true;
        spawn_npc_id = -1;
        spawn_npc_n = 0;
    }
};



struct v1_st_sprite_data {
    bool used;                                                  // if true, the sprite exists
    Uint16 duration;
    Uint8 sprite_graphic_pos_x;
    v1_st_rectangle colision_rect;
    v1_st_sprite_data() {
        used = false;
        duration = 0;
        sprite_graphic_pos_x = 0;
    }

    v1_st_sprite_data & operator=(v1_st_sprite_data new_value) {
        used = new_value.used;
        duration = new_value.duration;
        sprite_graphic_pos_x = new_value.sprite_graphic_pos_x;
        colision_rect = new_value.colision_rect;
        return *this;
    }
};


struct v1_file_weapon_colors { // DONE -> Player
    v1_st_color color1;
    v1_st_color color2;
    v1_st_color color3;
};

struct v1_file_player { // DONE -> Game
    char name[V1_CHAR_NAME_SIZE];
    char graphic_filename[V1_CHAR_FILENAME_SIZE];
    char face_filename[V1_CHAR_FILENAME_SIZE];
    Uint8 HP;
    v1_st_size_int8 sprite_size;                                       // size of sprite graphic
    v1_st_rectangle sprite_hit_area;                                   // the area of the graphic where is used for hit/colision
    Uint8 move_speed;                                               // how many sprites move each step
    v1_st_sprite_data sprites[V1_ANIM_TYPE_COUNT][V1_ANIM_FRAMES_COUNT];
    v1_file_weapon_colors weapon_colors[V1_MAX_WEAPON_N];
    v1_st_color color_keys[3];
    // habilities part
    bool have_shield;
    Uint8 max_shots;                                                // number of maximum simultaneous projectiles
    Uint8 simultaneous_shots;                                       // number of projectiles shot at one button press
    bool can_double_jump;
    bool can_slide;                                                 // if false, dashes instead of sliding
    bool can_charge_shot;
    Uint8 full_charged_projectile_id;
    /// NEW IN FILE-FORMAT 3.0
    bool can_air_dash;
    Sint8 damage_modifier;
    bool can_shot_diagonal;

    v1_file_player() {
        sprintf(name, "%s", "Player");
        graphic_filename[0] = '\0';
        face_filename[0] = '\0';
        HP = 0;
        move_speed = 2.0;
        max_shots = 3;
        simultaneous_shots = 3;
        can_double_jump = false;
        have_shield = false;
        can_slide = false;
        can_charge_shot = false;
        full_charged_projectile_id = 0;
        can_air_dash = false;
        damage_modifier = 0;
        can_shot_diagonal = false;
    }
};







struct v1_file_weapon { // DONE - Game
    Uint8 id_projectile;
    char name[V1_CHAR_NAME_SIZE];
    Uint8 damage;
    v1_file_weapon() {
        sprintf(name, "%s", "Weapon");
        damage = 1;
        id_projectile = 0;
    }
};

struct v1_file_object { // DONE - Game
    char name[V1_CHAR_NAME_SIZE];
    char graphic_filename[V1_CHAR_FILENAME_SIZE];
    Sint8 type;													///@ TODO: use enum
    int timer;													// used as time for disapearing block
    Sint8 speed;												// used as speed for moving platform
    Sint16 limit;												// used as range (pixels) for moving platform, visible time for disapearing block
    Sint8 direction;											// used to check if moving away from oiginalpoint or moving to it
    Sint16 distance;
    v1_st_size_int8 size;
    Sint16 frame_duration;										// indicates what is the used frame
    bool animation_auto_start;                                  // false means it will not be animated until started
    bool animation_reverse;                                     // start animating
    bool animation_loop;                                        // true = animation will loop, false = animation will run once
    v1_file_object() {
        sprintf(name, "%s", "Object");
        graphic_filename[0] = '\0';
        animation_auto_start = true;
        animation_reverse = false;
        animation_loop = true;
        frame_duration = 100;
        type = -1;
    }
};


struct v1_st_weakness {
    Sint8 weapon_id;                                           // weapon number of weakness, if -1, no weakness
    Sint8 damage_multiplier;                                   // how many time more damage is taken, if zero, no damage from this weapon is taken
    v1_st_weakness() {
        weapon_id = -1;
        damage_multiplier = 1;
    }
};


struct v1_file_npc { // DONE - Game
    Sint8 id;                                                  // serial number
    //unsigned int projectile_id;                               // indicates the projectile ID (relation with file_projectile)
    Sint8 projectile_id[2];                                    // indicate the id of an attack the NCP can user
    char name[V1_CHAR_NAME_SIZE];
    char graphic_filename[V1_CHAR_FILENAME_SIZE];
    struct v1_st_hit_points hp;
    Sint8 direction;                                           // defines how it behavies (kink of AI*)
    Sint8 speed;                                               // defines the distances it can see enemies
    Sint16 walk_range;                                         // defines how long from the start point it can go
    Sint8 facing;                                              // defines the side npc is facing before start moving (also used by LINEWALK behavior)
    v1_st_position start_point;
    v1_st_sprite_data sprites[V1_ANIM_TYPE_COUNT][V1_ANIM_FRAMES_COUNT]; // changed in 2.0.4
    v1_st_size_int8 frame_size;
    bool is_ghost;
    Sint8 shield_type;
    Sint8 IA_type;                                             // IA types. For custom (edited) ones, use IA_TYPES_COUNT + position (in the array)
    Sint8 fly_flag;                                            // 0 - can't fly, 1 - flyer (...)
    v1_st_weakness weakness[V1_FS_NPC_WEAKNESSES];                                    // each npc have weaknesses for each boss weapon, plus the normal weapon
    char bg_graphic_filename[V1_CHAR_FILENAME_SIZE];               // holds a static background
    v1_st_position sprites_pos_bg;                                 // holds position of sprites in relation with background
    bool is_boss;                                               // indicates if this NPC is a boss
    Sint8 attack_frame_n[V1_ANIM_TYPE_COUNT];                      // tells wich is the frame that ignites the attack
    bool is_sub_boss;                                           // a middle-stage boss, the doors will only open after it's dead
    int respawn_delay;                                          // if > 0, will respawn even if on-screen

    v1_file_npc() {
        id = -1;
        sprintf(name, "%s", "NPC");
        graphic_filename[0] = '\0';
        projectile_id[0] = -1;
        projectile_id[1] = -1;
        bg_graphic_filename[0] = '\0';
        walk_range = 160;
        fly_flag = 0;
        IA_type = 0;
        is_boss = false;
        speed = 3;
        is_sub_boss = false;
        for (int i=0; i<V1_ANIM_TYPE_COUNT; i++) {
            attack_frame_n[i] = -1;
        }
        respawn_delay = 0;
    }

};





struct v1_file_ai_action {
    int chance;
    int action;
    int go_to;                                                  // action number to be executed once this one is finished, -1 indicates that must return to CHANCES (random)
    int go_to_delay;                                            // delay before going to the next action
    int extra_parameter;										// left, right, etc
    v1_file_ai_action()
    {
        chance = 0;
        action = 0;
        go_to = 0;
        go_to_delay = 500;
        extra_parameter = 0;
    }
};

struct v1_reaction {
    int action;
    int go_to;                                                  // action number to be executed once this one is finished, -1 indicates that must return to CHANCES (random)
    int go_to_delay;                                            // delay before going to the next action
    int extra_parameter;										// left, right, etc
    v1_reaction() {
        action = -1;
        go_to = 0;
        go_to_delay = 500;
        extra_parameter = 0;
    }
};

struct v1_file_artificial_inteligence {
    char name[V1_CHAR_NAME_SIZE];
    v1_file_ai_action states[V1_AI_MAX_STATES];
    v1_reaction reactions[V1_MAX_AI_REACTIONS];                    // reactions near, hit and dead

    v1_file_artificial_inteligence() {
        sprintf(name, "%s", "A.I.");
    }
};







struct v1_st_checkpoint {
    int map; /**< TODO */
    int x; /**< TODO */
    int y; /**< TODO */
    int map_scroll_x; /**< TODO */
    v1_st_checkpoint() {
            map = 0;
            x = 50;
            y = -1;
            map_scroll_x = 0;
    }
};

struct v1_file_intro {
    int initial_delay;                                  // delay before showing text and effects
    char bg_filename[V1_FS_CHAR_FILENAME_SIZE];            // background image
    int transition_effect;                              // how we transition to next screen
};

struct v1_st_shop_dialog {
    char face_graphics_filename[20];
    char line1[V1_FS_DIALOG_LINES][V1_DIALOG_LINE_LIMIT];
};


enum v1_e_trophy_conditions {
    V1_TROPHY_CONDITIONS_NO_DAMAGE,                                    // finish a stage without being hit
    V1_TROPHY_CONDITIONS_END_GAME,                                     // finish the game
    V1_TROPHY_CONDITIONS_KILLER,                                       // kill 1000 enemies @TODO - add counter to save
    V1_TROPHY_CONDITIONS_NO_CONTINUES,                                 // finish the game without using any continue @TODO - add flag in save
    V1_TROPHY_CONDITIONS_COUNT
};

struct v1_st_file_trophy {
    Uint8 condition;
    char name[V1_FS_NAME_SIZE];
    char filename[V1_FS_FILENAME_SIZE];
};


struct v1_st_armor_piece {
    int special_ability[V1_FS_MAX_PLAYERS]; // each player can have a different ability
    char got_message[V1_FS_MAX_PLAYERS][V1_FS_DIALOG_LINES][V1_DIALOG_LINE_LIMIT];
    Sint8 projectile_id;                // for armor-arms you can get a different attack, defined here

    v1_st_armor_piece() {
        for (int i=0; i<V1_FS_MAX_PLAYERS; i++) {
            special_ability[i] = 0;
            for (int j=0; j<V1_FS_DIALOG_LINES; j++) {
                got_message[i][j][0] = '\0';
            }
        }
        projectile_id = -1;
    }
};


struct v1_file_game {
    float version;
    char name[V1_FS_CHAR_NAME_SIZE];
    v1_file_npc game_npcs[V1_FS_GAME_MAX_NPCS];                           // 60 enemy types is more than good enought
    v1_file_object objects[V1_FS_GAME_MAX_NPCS];
    v1_file_projectile projectiles[V1_FS_MAX_PROJECTILES];
    v1_file_weapon weapons[V1_FS_MAX_WEAPONS];                            // 8 stage-bosses and 3 item-like
    v1_file_player players[V1_FS_MAX_PLAYERS];                            // up to 4 different players the user can select from
    v1_file_artificial_inteligence ai_types[V1_FS_MAX_AI_TYPES];
    Sint8 semi_charged_projectile_id;                              // common to all players
    Sint8 player_items[V1_FS_PLATER_ITEMS_N];                                         // common to all players -> to be used in add_coil_object and add_jet_object
    char stage_face_filename[V1_MAX_STAGES][V1_FS_FACE_FILENAME_MAX];
    v1_st_file_trophy trophies[V1_TROPHIES_MAX];
    v1_st_shop_dialog shop_dialog_welcome;
    v1_st_shop_dialog shop_dialog_goodbye;
    v1_st_armor_piece armor_pieces[V1_FS_PLAYER_ARMOR_PIECES_MAX];
    v1_st_anim_map_tile anim_tiles[V1_FS_ANIM_TILES_MAX];


    // CONSTRUCTOR //
    v1_file_game() {
        version = 3.00; // file-format version, not game
        sprintf(name, "%s", "Rockbot");
        semi_charged_projectile_id = 0;
        player_items[0] = 0;
        player_items[1] = 0;
        for (int i=0; i<V1_MAX_STAGES; i++) {
            stage_face_filename[i][0] = '\0';
        }
        for (int i=0; i<V1_FS_GAME_MAX_OBJS; i++) {
            sprintf(game_npcs[i].name, "NPC [%d]", i);
        }
        for (int i=0; i<V1_FS_GAME_MAX_OBJS; i++) {
            sprintf(objects[i].name, "Object [%d]", i);
        }
        for (int i=0; i<V1_FS_MAX_PROJECTILES; i++) {
            sprintf(projectiles[i].name, "Projectile [%d]", i);
        }
        for (int i=0; i<V1_FS_MAX_WEAPONS; i++) {
            sprintf(projectiles[i].name, "Projectile [%d]", i);
        }
        for (int i=0; i<V1_FS_MAX_PLAYERS; i++) {
            sprintf(players[i].name, "Player [%d]", i);
        }
        for (int i=0; i<V1_FS_MAX_AI_TYPES; i++) {
            sprintf(ai_types[i].name, "A.I. [%d]", i);
        }
    }
};



struct v1_st_map_background {
    char filename[V1_FS_CHAR_FILENAME_SIZE];
    Sint16 adjust_y;                             // distance of the background from the screen top
    Sint8 speed;                                // 0 means will scroll in the same speed as foreground (so the bg is used only to ease map-creation)
    bool auto_scroll;                           // automatic move background even if player isn't moving
    v1_st_map_background()
    {
        filename[0] = '\0';
        adjust_y = 0;
        speed = 0;
        auto_scroll = false;
        sprintf(filename, "%s", "");
    }
};

struct v1_map_tile {
    Sint8 locked;
    v1_st_position_int8 tile1;                  // position of tile (x and y) in the graphic file (each tile is 16*16)
    v1_st_position_int8 tile3;                  // tird level, to be shown over player
    v1_map_tile() {
        locked = 0;
        tile1.x = -1;
        tile1.y = -1;
        tile3.x = -1;
        tile3.y = -1;
    }
};



struct v1_file_map_npc {
    Sint8 id_npc;
    v1_st_position start_point;
    Uint8 direction;
    v1_file_map_npc() {
        id_npc = -1;
        direction = 0;
    }
};

struct v1_file_map_object {
    Sint8 id_object;
    v1_st_position start_point;
    Uint8 direction;
    v1_st_position link_dest;                          // used for teleporter
    Sint8 map_dest;
    v1_file_map_object() {
        id_object = -1;
        direction = 0;
        map_dest = -1;
        link_dest.x = -1;
        link_dest.y = -1;
    }
};

struct v1_file_map {
    v1_st_map_background backgrounds[2];
    v1_st_color background_color;
    v1_map_tile tiles[V1_MAP_W][V1_MAP_H];		// map tiles
    v1_file_map_npc map_npcs[V1_FS_MAX_MAP_NPCS];
    v1_file_map_object map_objects[V1_FS_MAX_MAP_OBJECTS];
    v1_file_map() {
        for (int i=0; i<V1_FS_MAX_MAP_NPCS; i++) {
            map_npcs[i].id_npc = -1;
        }
        for (int i=0; i<V1_FS_MAX_MAP_OBJECTS; i++) {
            map_objects[i].id_object = -1;
        }
        // add ground to new map
        for (int i=0; i<V1_MAP_W; i++) {
            tiles[i][V1_MAP_H-2].locked = V1_TERRAIN_SOLID;
            tiles[i][V1_MAP_H-1].locked = V1_TERRAIN_SOLID;
        }
    }
};





struct v1_file_dialog { // DONE - Stage x2
    char face_graphics_filename[20];
    bool top_side;
    char line1[V1_FS_DIALOG_LINES][V1_DIALOG_LINE_LIMIT];
    char answer1[V1_FS_MAX_PLAYERS][V1_FS_DIALOG_LINES][V1_DIALOG_LINE_LIMIT];
    char line2[V1_FS_DIALOG_LINES][V1_DIALOG_LINE_LIMIT];
    char answer2[V1_FS_MAX_PLAYERS][V1_FS_DIALOG_LINES][V1_DIALOG_LINE_LIMIT];
    v1_file_dialog() {
        top_side = true;
        face_graphics_filename[0] = '\0';
        for (int i=0; i<V1_FS_MAX_PLAYERS; i++) {
            for (int j=0; j<3; j++) {
                answer1[i][j][0] = '\0';
                answer2[i][j][0] = '\0';
            }
        }
        for (int j=0; j<3; j++) {
            line1[j][0] = '\0';
            line2[j][0] = '\0';
        }
    }
};

// defines a castle intro and details
struct v1_file_castle {
    v1_st_position points[V1_CASTLE_STAGES_MAX];
    char filename[V1_FS_FILENAME_SIZE];                                        // castle background image
    v1_file_dialog boss_defeated_dialog;                                       // boss defeated dialog
};




struct file_link { // DONE - Stage
    Sint8 id_map_origin;
    Sint8 id_map_destiny;
    v1_st_position_uint8 pos_origin;
    v1_st_position_uint8 pos_destiny;
    Uint8 size;
    bool bidirecional;
    bool is_door;
    Sint8 type;
    file_link() {
        id_map_origin = -1;
        id_map_destiny = -1;
        size = 1;
        bidirecional = false;
        is_door = false;
        type = V1_LINK_VERTICAL;
    }
};

struct v1_file_boss { // DONE - Stage
    Sint8 id_npc;
    Sint8 id_weapon;
    char name[20];
    char face_graphics_filename[20];
    v1_file_boss() {
        id_npc = -1;
        id_weapon = -1;
        sprintf(name, "%s", "Boss");
        face_graphics_filename[0] = '\0';
    }
};


struct v1_file_colorcycle { // DONE - Stage
    v1_st_color color[V1_FS_COLORCYCLE_MAX_ITEMS][V1_COLOR_KEY_LENGTH];
    Sint16 duration[V1_FS_COLORCYCLE_MAX_ITEMS][V1_COLOR_KEY_LENGTH];
};

struct v1_file_stage {
    char name[V1_FS_CHAR_NAME_SIZE];
    char bgmusic_filename[V1_FS_CHAR_FILENAME_SIZE];
    v1_file_map maps[V1_FS_STAGE_MAX_MAPS];
    file_link links[V1_FS_STAGE_MAX_LINKS];
    v1_file_boss boss;
    v1_file_dialog intro_dialog;
    v1_file_dialog boss_dialog;
    v1_file_colorcycle colorcycle;

    // CONSTRUCTOR //
    v1_file_stage() {
       boss = v1_file_boss();
        sprintf(name, "%s", "Stage");
        bgmusic_filename[0] = '\0';
    }
};


struct v1_file_stages {
    v1_file_stage stages[V1_FS_MAX_STAGES];
    v1_file_stages() {
        for (int i=0; i<V1_FS_MAX_STAGES; i++) {
            sprintf(stages[i].name, "Stage [%d]", i);
        }
    }
};







#endif // V1_H

