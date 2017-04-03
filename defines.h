#ifndef DEFINES_H
#define DEFINES_H

#define FRAMES_PER_SECOND 60

#define RES_W 320
#define RES_H 240

#define UNUSED(x) (void)x

enum e_PLAYERS {
    PLAYER_1,
    PLAYER_2,
    PLAYER_3,
    PLAYER_4
};

#define WALK_FRAME_DELAY 150

// ==================== PHYSICS ==================== //









// stages
enum STAGE_LIST {
    INTRO_STAGE, STAGE1, STAGE2, STAGE3, STAGE4, STAGE5, STAGE6, STAGE7, STAGE8, CASTLE1_STAGE1, CASTLE1_STAGE2, CASTLE1_STAGE3, CASTLE1_STAGE4, CASTLE1_STAGE5, STAGE_COUNT
};


/**
 * @brief
 *
 */
enum COLlISION_TYPES { BLOCK_UNBLOCKED, BLOCK_X, BLOCK_Y, BLOCK_XY, BLOCK_STAIR_X, BLOCK_STAIR_Y, BLOCK_WATER, BLOCK_MOVE_LEFT, BLOCK_MOVE_RIGHT, BLOCK_INSIDE_OBJ };

/**
 * @brief
 *
 */
enum JUMP_STATES { NO_JUMP, JUMP_UP, JUMP_DOWN };

#define TIME_MOVES 0

enum E_INPUT_TYPES {
    INPUT_TYPE_DOUBLE,
    INPUT_TYPE_KEYBOARD,
    INPUT_TYPE_JOYSTICK
};

/**
 * @brief
 *
 */
enum E_INPUT_MODES {
	INPUT_MODE_DOUBLE, // uses both keyboard and joystick
    INPUT_MODE_ANALOG,
    INPUT_MODE_DIGITAL
};

enum E_PLATFORM {
    PLATFORM_PS2,
    PLATFORM_WII,
    PLATFORM_PSP,
    PLATFORM_LINUX,
    PLATFORM_WINDOWS,
    PLATFORM_DINGUX,
    PLATFORM_PANDORA,
    PLATFORM_DREAMCAST,
    PLATFORM_COUNT
};


/**
 * @brief
 *
 */
enum INPUT_COMMANDS {
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_JUMP,
    BTN_ATTACK,
    BTN_QUIT,
    BTN_START,
    BTN_L,
    BTN_R,
    BTN_DASH,
    BTN_SHIELD,
    BTN_JUMP_TIMER,
    BTN_COUNT
};


enum joystick_input_types {
    JOYSTICK_INPUT_TYPE_NONE,
    JOYSTICK_INPUT_TYPE_BUTTON,
    JOYSTICK_INPUT_TYPE_HAT,
    JOYSTICK_INPUT_TYPE_AXIS,
    JOYSTICK_INPUT_TYPE_COUNT
};


enum ANIM_DIRECTION {
    ANIM_DIRECTION_LEFT,
    ANIM_DIRECTION_RIGHT,
    ANIM_DIRECTION_UP,
    ANIM_DIRECTION_DOWN,
    ANIM_DIRECTION_UP_LEFT,
    ANIM_DIRECTION_UP_RIGHT,
    ANIM_DIRECTION_DOWN_LEFT,
    ANIM_DIRECTION_DOWN_RIGHT,
    ANIM_DIRECTION_COUNT };


#define CHAR_ANIM_DIRECTION_COUNT 2 // characters use only left/right, not like projectiles

/**
 * @brief
 *
 */
enum ANIM_TYPE {
    ANIM_TYPE_STAND,
    ANIM_TYPE_WALK,
    ANIM_TYPE_JUMP,
    ANIM_TYPE_ATTACK,
    ANIM_TYPE_TELEPORT,
    ANIM_TYPE_JUMP_ATTACK,
    ANIM_TYPE_WALK_ATTACK,
    ANIM_TYPE_HIT,
    ANIM_TYPE_STAIRS,
    ANIM_TYPE_STAIRS_SEMI,
    ANIM_TYPE_STAIRS_ATTACK,
    ANIM_TYPE_STAIRS_MOVE,
    ANIM_TYPE_THROW,
    ANIM_TYPE_TURN,
    ANIM_TYPE_MOVE_SEMI, // like turn, the start of the movement
    ANIM_TYPE_WALK_AIR, // for those that can walk on air
    ANIM_TYPE_INTRO, // for bosses
    ANIM_TYPE_SPECIAL_ATTACK, // for bosses, charging weapon, etc
    ANIM_TYPE_SLIDE,
    ANIM_TYPE_SHIELD,
    ANIM_TYPE_VERTICAL_TURN,
    ANIM_TYPE_ATTACK_THROW,
    ANIM_TYPE_ATTACK_DIAGONAL_UP,
    ANIM_TYPE_ATTACK_DIAGONAL_DOWN,
    ANIM_TYPE_ATTACK_UP,
    ANIM_TYPE_ATTACK_DOWN,
    ANIM_TYPE_ATTACK_SPECIAL, // (hadouken, shouryuken, etc)
    ANIM_TYPE_LOOK_UP,
    ANIM_TYPE_GRAB_WALL,
    ANIM_TYPE_COUNT
};

// alguns como attack_trhow e throw; e special attack e attack_special estão repetidos. fica de reserva futura


#define ANIM_FRAMES_COUNT 10 // max number of animation frames for each animation_type

#define MAP_W 200 // this size was determined by looking at common maps from games
#define MAP_H 15
#define FORMAT202_MAP_H 13

#define TILESIZE 16

#define GRAVITY_MAX_SPEED 8
#define GRAVITY_TELEPORT_MAX_SPEED 12

// for pre-loading


/**
 * @brief
 *
 */
enum STAGE_SELECT_ENUM {
	INTRO_SURFACES_STAGE_SELECT_BG,
	STAGE_SELECT_COUNT
};

// sound
/**
 * @brief
 *
 */
enum SFX_LIST {
	SFX_NPC_HIT,
	SFX_NPC_KILLED,
	SFX_PLAYER_HIT,
	SFX_PLAYER_SHOT,
	SFX_PLAYER_JUMP,
	SFX_CURSOR,
	SFX_STAGE_SELECTED,
	SFX_GOT_ENERGY,
	SFX_GOT_ITEM,
	SFX_SHOT_REFLECTED,
	SFX_DOOR_OPEN,
	SFX_GOT_WEAPON,
	SFX_TELEPORT,
	SFX_IMPLOSION,
	SFX_PLAYER_DEATH,
	SFX_DESTRIN_SHIP,
	SFX_SKULL_CASTLE_INTRO,
	SFX_PLAYER_CHARGED_SHOT,
	SFX_CHARGING1,
	SFX_CHARGING2,
	SFX_BIG_EXPLOSION,
	SFX_WATER_ENTER,
	SFX_WATER_LEAVE,
    SFX_DISAPPEARING_BLOCK,
    SFX_SHORYUKEN_GIRL,
    SFX_BEAM,
    SFX_GOT_ENERGY_BIG,
	SFX_COUNT // not used as sfx, this is a way to measure size of the enum list
};

/**
 * @brief
 *
 */
enum TERRAIN_TYPES {
	TERRAIN_UNBLOCKED,
	TERRAIN_SOLID,
	TERRAIN_STAIR,
	TERRAIN_DOOR,
	TERRAIN_SPIKE,
	TERRAIN_WATER,
	TERRAIN_ICE,
	TERRAIN_MOVE_LEFT,
	TERRAIN_MOVE_RIGHT,
	TERRAIN_SAND,
	TERRAIN_SCROLL_LOCK,
	TERRAIN_CHECKPOINT,
    TERRAIN_EASYMODEBLOCK,
    TERRAIN_HARDMODEBLOCK,
	TERRAIN_TYPES_COUNT
};


#define CURSOR_SPACING 12

#define NPC_FRAME_NUMBER 50


/**
 * @brief
 *
 */
enum ITEM_ICONS_ENUM {
	ENERGY_TANK,
	P1_1LIFE,
	P2_1LIFE,
	WEAPONS_TANK
};



enum WEAPON_ICONS_ENUM {
    WEAPON_DEFAULT,
    WEAPON_APEBOT,
    WEAPON_DAISIEBOT,
    WEAPON_DYNAMITEBOT,
    WEAPON_MUMMYBOT,
    WEAPON_SPIKEBOT,
    WEAPON_TECHNOBOT,
    WEAPON_MAGEBOT,
    WEAPON_SEAHORSEBOT,
    WEAPON_ITEM_COIL,
    WEAPON_ITEM_JET,
	WEAPON_COUNT
};

/**
 * @brief
 *
 */
enum transition_types {
	NO_TRANSITION,
	TRANSITION_TOP_TO_BOTTOM,
	TRANSITION_BOTTOM_TO_TOP,
	TRANSITION_LEFT_TO_RIGHT,
	TRANSITION_RIGHT_TO_LEFT
};

/**
 * @brief
 *
 */
enum OBJECT_TYPE {
	OBJ_MOVING_PLATFORM_UPDOWN,
	OBJ_MOVING_PLATFORM_LEFTRIGHT,
	OBJ_DISAPPEARING_BLOCK,
	OBJ_ENERGY_TANK,
	OBJ_WEAPON_TANK,
	OBJ_ENERGY_PILL_BIG,
	OBJ_WEAPON_PILL_BIG,
	OBJ_ENERGY_PILL_SMALL,
	OBJ_WEAPON_PILL_SMALL,
	OBJ_LIFE,
	OBJ_FALL_PLATFORM,
	OBJ_FLY_PLATFORM,
	OBJ_ITEM_FLY,
	OBJ_ITEM_JUMP,
    OBJ_ACTIVE_DISAPPEARING_BLOCK, // disappear only after player activates it
    OBJ_ARMOR_ARMS,
    OBJ_ARMOR_BODY,
    OBJ_ARMOR_LEGS,
    OBJ_RAY_HORIZONTAL,
    OBJ_RAY_VERTICAL,
    OBJ_TRACK_PLATFORM,
    OBJ_BOSS_TELEPORTER,
    OBJ_DESTRUCTIBLE_WALL,
    OBJ_SPECIAL_TANK,
    OBJ_DEATHRAY_VERTICAL,
    OBJ_DEATHRAY_HORIZONTAL,
    OBJ_FINAL_BOSS_TELEPORTER,
    OBJ_ACTIVE_OPENING_SLIM_PLATFORM,
    OBJ_DAMAGING_PLATFORM,
	OBJ_TYPE_COUNT
};

/**
 * @brief
 *
 */
enum OBJ_STATE {
	OBJ_STATE_STAND,
	OBJ_STATE_MOVE,
	OBJ_STATE_RETURN,
	OBJ_STATE_COUNT
};

/**
 * @brief
 *
 */
enum LINK_TYPES { LINK_HORIZONTAL, LINK_VERTICAL, LINK_TELEPORTER, LINK_FADE_TELEPORT, LINK_TELEPORT_LEFT_LOCK, LINK_TELEPORT_RIGHT_LOCK, LINK_TYPE_COUNT };

#define DOOR_MOVE_STEP 2

#define TRANSITION_STEP 4

#define PLAYER_INITIAL_HP 28

#define BOSS_INITIAL_HP 36 // a bit more HP (8 pts) than player, so it will endure a bit more on the fight

#define HIT_BLINK_ANIMATION_LAPSE 100

#define WATER_SPEED_MULT 0.9

#define ATTACK_DELAY 300 // how many milisseconds the attack frame must last before returning to stand/jump/walk

/**
 * @brief
 *
 */
enum PROJECTILE_TRAJECTORIES {
	TRAJECTORY_LINEAR,
	TRAJECTORY_ARC,
	TRAJECTORY_SIN,
	TRAJECTORY_CHAIN,
	TRAJECTORY_QUAKE,
	TRAJECTORY_BOMB,
	TRAJECTORY_FOLLOW,
	TRAJECTORY_PROGRESSIVE,
	TRAJECTORY_FREEZE,
	TRAJECTORY_DIAGONAL_UP,
	TRAJECTORY_DIAGONAL_DOWN,
    TRAJECTORY_CENTERED,                // centered around character
    TRAJECTORY_ZIGZAG,                  // linear until reach a wall, then return (repeat until reflection number = 3)
    TRAJECTORY_TARGET_DIRECTION,        // adjust to linear, diagonal up or diagonal down depending on player position when shoot
    TRAJECTORY_ARC_TO_TARGET,           // forms an arn that will end at player's position
    TRAJECTORY_TARGET_EXACT,            // will go exactly to the point the target is in
    TRAJECTORY_FALL_BOMB,               // falls until ground then explodes
    TRAJECTORY_LASER,
    TRAJECTORY_PUSH_BACK,               // pushes player or enemies into opposite direction
    TRAJECTORY_ARC_SMALL,
    TRAJECTORY_RING,                    // increases size and on last frame, oponent can pass in the middle
    TRAJECTORY_LIGHTING,                // appears sequentially in some parts of the screen
	PROJECTILE_TRAJECTORIES_COUNT };


/**
 * @brief
 *
 */
enum EDITOR_MODES {
	EDITMODE_NORMAL,
	EDITMODE_LOCK,
	EDITMODE_ERASER,
	EDITMODE_FILL,
	EDITMODE_LINK,
	EDITMODE_LINK_DEST,
	EDITMODE_NPC,
	EDITMODE_ADDNPC,
    EDITMODE_OBJECT,
    EDITMODE_SET_BOSS,
    EDITMODE_SET_SUBBOSS,
    EDITMODE_ANIM_TILE,
    EDITMODE_OBJECT_LINK_PLACING,
    EDITMODE_SELECT,
    EDITMODE_PASTE
};


/**
 * @brief
 *
 */
enum IA_TYPE_LIST {
    IA_STAND,
    IA_WAIT,
    IA_FOLLOW,
    IA_ZIGZAG,
    IA_SIDETOSIDE,
    IA_BAT,
    IA_ROOF_SHOOTER,
    IA_GROUND_SHOOTER,
    IA_SHOOT_AND_GO,
    IA_FLY_ZIG_ZAG,
    IA_BUTTERFLY,
    IA_HORIZONTAL_GO_AHEAD,
    IA_HORIZONTAL_TURN,
    IA_FIXED_JUMPER,
    IA_SIDE_SHOOTER,
    IA_GHOST,
    IA_FISH,
    IA_DOLPHIN,
    IA_VERTICAL_ZIGZAG, IA_TYPES_COUNT };


/**
 * @brief
 *
 */
enum IA_STATES {
		IA_INITIAL_STATE,
		IA_STATE_SLEEP,
		IA_STATE_FOLLOW,
		IA_STATE_RETURN,
		IA_STATE_SHOT_RAINBOW,
		IA_STATE_CROSS_SCREEN,
		IA_STATE_SHOT_FLOWER,
		IA_STATE_WAIT_TIME,
		IA_STATE_START_MOVE,
		IA_STATE_START_STAND,
		IA_STATE_INTRO,
		IA_STATE_JUMP_TO_ROOF,
		IA_STATE_JUMP_TO_PLAYER,
		IA_STATE_AIR_WALK,
		IA_STATE_QUAKE_ATTACK,
		IA_STATE_JUMP_FALL,
		IA_STATE_GROUND_ATTACK,
		IA_STATE_TELEPORT,
		IA_STATE_WALK_TO_POINT,
		IA_STATE_CHARGED_DASH,
		IA_STATE_SHOT_STOP_TIME,
		IA_STATE_JUMP_TO_WALL,
		IA_STATE_FLOAT_TO_POINT,
		IA_STATE_COUNT
};

/**
 * @brief
 *
 */
enum IA_CONDITIONALS {
	IA_CONDITION_NO_CONDITION,
	IA_CONDITION_NEAR_PLAYER,
	IA_CONDITION_ABOUT_TO_DIE,
	IA_CONDITION_ABOVE_PLAYER,
	IA_CONDITION_UNDER_PLAYER
};

#define AI_MAX_STATES 20
#define MAX_AI_REACTIONS 6


/**
 * @brief
 *
 */
enum e_shield_types { SHIELD_NO, SHIELD_FULL, SHIELD_FRONT, SHIELD_STAND, SHIELD_DISGUISE, SHIELD_STAND_FRONT, SHIELD_COUNT };

/**
 * @brief
 *
 */
enum e_energy_types { ENERGY_TYPE_HP, ENERGY_TYPE_WEAPON };


#define MAX_PLAYER_SPRITES 23
#define MAX_NPC_SPRITES 15
#define MAX_WEAPON_N 12
#define MAX_STAGES 16
#define MAX_MAP_NPC_N 30
#define STAGE_MAX_LINKS 30
#define STAGE_MAX_MAPS 3

#define CHAR_NAME_SIZE 30

#define MAX_FILE_PLAYERS 5

#define MAX_FILE_PROJECTILES 20

#define PROJECTILE_DEFAULT_SPEED 6
#define PROJECTILE_DEFAULT_DAMAGE 1
#define PROJECTILE_DEFAULT_ANIMATION_TIME 100

#define DIALOG_LINE_LIMIT 30
#define EDITOR_DIALOG_LINE_LIMIT 27
#define MAX_PLAYERS 3

#ifdef PLAYSTATION2
    #define JOYVAL 12000
#else
	#define JOYVAL 30000
#endif

// ATUAL: []: tiro, X: pulo, O: dash =>
// DESEJADO: X: ataque, O: pulo, /\: dash

#define ENERGY_ITEM_SMALL 3
#define ENERGY_ITEM_BIG 8

#define MOVING_GROUND 10 //TREADMILL


#define CHARGED_SHOT_INITIAL_TIME 1000
#define CHARGED_SHOT_TIME 2200
#define SUPER_CHARGED_SHOT_TIME 3200
#define TURBO_ATTACK_INTERVAL 200

#define TOUCH_DAMAGE_SMALL 2
#define TOUCH_DAMAGE_BIG 4

// from graphiLib
#define WPN_COLUMN1_X 42
#define WPN_COLUMN2_X 182
#define WPN_COLUMN_Y 53

#define _MIXER_THREAD_PRIORITY 3


#define PLAYER_NPC_COLLISION_REDUTOR 6


/**
 * @brief
 *
 */
enum e_game_flags {
	FLAG_QUICKLOAD,
	FLAG_INVENCIBLE,
	FLAG_INFINITE_JUMP,
	FLAG_ALLWEAPONS,
    FLAG_INFINITE_HP,
    FLAG_PLAYER1,
    FLAG_PLAYER2,
    FLAG_PLAYER3,
    FLAG_PLAYER4,
	FLAG_COUNT
};


enum AI_ACTION_LIST {
	AI_ACTION_WALK,
	AI_ACTION_FLY,
	AI_ACTION_JUMP,
	AI_ACTION_WAIT_UNTIL_PLAYER_IS_IN_RANGE,
	AI_ACTION_SAVE_POINT,
    AI_ACTION_SHOT_PROJECTILE_AHEAD,
    AI_ACTION_SHOT_PROJECTILE_PLAYER_DIRECTION,
    AI_ACTION_SHOT_PROJECTILE_INVERT_DIRECTION,
	AI_ACTION_AIR_WALK,
	AI_ACTION_FALL_TO_GROUND,
	AI_ACTION_TELEPORT,
	AI_ACTION_DASH,
	AI_ACTION_GRAB_WALL,
    AI_ACTION_SPAWN_NPC,
    AI_ACTION_CHANGE_MOVE_TYPE,
    AI_ACTION_REPLACE_NPC,                          // similar to spawn npc, but removes itself
    AI_ACTION_CIRCLE_PLAYER,
    AI_ACTION_CHANGE_MOVE_TYPE_REVERSE,
    AI_ACTION_JUMP_ATTACK_UP,
    AI_ACTION_JUMP_ATTACK_AHEAD_ONCE,
    AI_ACTION_WAIT_RANDOM_TIME,
	AI_ACTION_LIST_SIZE
};


enum AI_ACTION_SHOT_OPTIONS {
    AI_ACTION_SHOT_FACEPLAYER,
    AI_ACTION_SHOT_AHEAD
};


enum AI_ACTION_GOTO_LIST {
	AI_ACTION_GOTO_CHANCE,
	AI_ACTION_GOTO_1,
	AI_ACTION_GOTO_2,
	AI_ACTION_GOTO_3,
	AI_ACTION_GOTO_4
};


/**
 * @brief
 *
 */
enum AI_ACTION_WALK_OPTION_LIST {
    AI_ACTION_WALK_OPTION_HORIZONTAL_AHEAD,
    AI_ACTION_WALK_OPTION_HORIZONTAL_TURN, // execute turn animation, ends when animation is compelted
    AI_ACTION_WALK_OPTION_TO_PLAYER,
    AI_ACTION_WALK_OPTION_TO_SAVED_POINT,
    AI_ACTION_WALK_OPTION_TURN_TO_PLAYER,
    AI_ACTION_WALK_OPTION_TO_RANDOM_DIRECTION,
    AI_ACTION_WALK_OPTION_TO_OPPOSITE_DIRECTION,

    AI_ACTION_WALK_OPTION_COUNT
};


/**
 * @brief
 *
 */
enum AI_ACTION_FLY_OPTION_LIST {
    AI_ACTION_FLY_OPTION_HORIZONTAL_AHEAD,
    AI_ACTION_FLY_OPTION_HORIZONTAL_TURN,
    AI_ACTION_FLY_OPTION_UP,
    AI_ACTION_FLY_OPTION_DOWN,
    AI_ACTION_FLY_OPTION_TO_PLAYER,
    AI_ACTION_FLY_OPTION_TO_SAVED_POINT,
    AI_ACTION_FLY_OPTION_TO_RANDOM_POINT,
    AI_ACTION_FLY_OPTION_FALL, // fall until out of screen or hitting ground (ignores distance)
    AI_ACTION_FLY_OPTION_DRILL_DOWN,
    AI_ACTION_FLY_OPTION_VERTICAL_CENTER,
    AI_ACTION_FLY_OPTION_PLAYER_DIRECTION,
    AI_ACTION_FLY_OPTION_OPOSITE_WALL,
    AI_ACTION_FLY_OPTION_OPOSITE_SHOOT_1,
    AI_ACTION_FLY_OPTION_TO_PLAYER_X, // move only in the X axis
    AI_ACTION_FLY_OPTION_TO_PLAYER_Y, // move only in the Y axis
    AI_ACTION_FLY_OPTION_ZIGZAG_AHEAD, // similar to horizontal-aahead but moving a bit on Y axias up/down
    AI_ACTION_FLY_OPTION_RANDOM_X,
    AI_ACTION_FLY_OPTION_RANDOM_Y
};

/**
 * @brief
 *
 */
enum AI_ACTION_JUMP_OPTION_LIST {
    AI_ACTION_JUMP_OPTION_AHEAD,
    AI_ACTION_JUMP_OPTION_TO_RANDOM_POINT,
    AI_ACTION_JUMP_OPTION_UP,
    AI_ACTION_JUMP_OPTION_TO_PLAYER,
    AI_ACTION_JUMP_OPTION_TO_ROOF,
    AI_ACTION_JUMP_OPTION_TO_SAVED_POINT,
    AI_ACTION_JUMP_OPTION_ONCE
};

/**
 * @brief
 *
 */
enum AI_ACTION_AIR_WALK_OPTION_LIST {
    AI_ACTION_AIR_WALK_OPTION_LEFT,
    AI_ACTION_AIR_WALK_OPTION_RIGHT,
    AI_ACTION_AIR_WALK_OPTION_TO_PLAYER
};

/**
 * @brief
 *
 */
enum AI_ACTION_TELEPORT_OPTION_LIST {
    AI_ACTION_TELEPORT_OPTION_LEFT,
    AI_ACTION_TELEPORT_OPTION_RIGHT,
    AI_ACTION_TELEPORT_OPTION_TO_PLAYER,
    AI_ACTION_TELEPORT_OPTION_RANDOM_X,
    AI_ACTION_TELEPORT_OPTION_RANDOM_Y,
    AI_ACTION_TELEPORT_OPTION_RANDOM_POINT,
    AI_ACTION_TELEPORT_OPTION_AHEAD
};

/**
 * @brief
 *
 */
enum AI_ACTION_DASH_OPTION_LIST {
    AI_ACTION_DASH_OPTION_LEFT,
    AI_ACTION_DASH_OPTION_RIGHT,
    AI_ACTION_DASH_OPTION_TO_PLAYER,
    AI_ACTION_DASH_OPTION_OPPOSITE_DIRECTION,
    AI_ACTION_DASH_OPTION_COUNT
};


/**
 * @brief
 *
 */
enum AI_ACTION_GRAB_WALL_OPTION_LIST {
    AI_ACTION_GRAB_WALL_OPTION_LEFT,
    AI_ACTION_GRAB_WALL_OPTION_RIGHT
};

/**
 * @brief
 *
 */
enum DROP_ITEMS_LIST {
    DROP_ITEM_1UP,
    DROP_ITEM_ENERGY_SMALL,
    DROP_ITEM_ENERGY_BIG,
    DROP_ITEM_WEAPON_SMALL,
    DROP_ITEM_WEAPON_BIG,
    DROP_ITEM_COUNT
};

#define COIL_GOT_STAGE 1
#define JET_GOT_STAGE 6

#define KEY_MAP_N 7


enum collision_modes {
    COLlISION_MODE_XY,
    COLlISION_MODE_X,
    COLlISION_MODE_Y
};


#define FONT_SIZE 8
#define FONT_SIZE_SMALL 8

#define MENU_CHANGE_DELAY 100

#define QUAKE_SCREEN_MOVE 2

enum FREEZE_EFFECT_TYPES {
    FREEZE_EFFECT_NONE,
    FREEZE_EFFECT_PLAYER,
    FREEZE_EFFECT_NPC
};

/**
 * @brief
 *
 */
enum COLOR_KEYS {
    COLOR_KEY_GREEN,
    COLOR_KEY_PURPLE,
    COLOR_KEY_CYAN,
    COLOR_KEY_LENGTH
};

enum e_VIDEO_FILTERS {
    VIDEO_FILTER_NOSCALE,
    VIDEO_FILTER_BITSCALE,
    VIDEO_FILTER_SCALE2x,
    VIDEO_FILTER_COUNT
};

#define BOSS_HIT_DURATION 800

#define COLORKEY_R 75
#define COLORKEY_G 125
#define COLORKEY_B 125


#define COLOR_COUNT 68

// filesystem defines
#define FS_MAX_WEAPONS 20
#define FS_MAX_PLAYERS 4
#define FS_MAX_STAGES 20

#define FS_CHAR_NAME_SIZE 30
#define FS_CHAR8_NAME_SIZE 8
#define FS_CHAR_FILENAME_SIZE 30
#define FS_INI_KEY_SIZE 30

#define FS_FILENAME_SIZE 30
#define FS_NAME_SIZE 30

#define FS_STAGE_MAX_LINKS 30
#define FS_STAGE_MAX_MAPS 3

#define FS_MAX_MAP_NPCS 30
#define FS_MAX_MAP_OBJECTS 30

#define FS_NPC_WEAKNESSES 9

#define FS_DIALOG_LINES 3

#define TROPHIES_MAX 10
#define CASTLE_STAGES_MAX 5

#define FS_COLOR_KEYS_N 3
#define FS_NPC_PROJECTILE_N 2


#define FS_PLAYER_ARMOR_PIECES_MAX 50 // allow more pieces in the future
enum e_ARMOR_PIECES {
    ARMOR_ARMS, ARMOR_BODY, ARMOR_LEGS
};

enum e_ARMOR_ABILITIES_ARMS {
    ARMOR_ABILITY_ARMS_SUPERSHOT, ARMOR_ABILITY_ARMS_LASERBEAM, ARMOR_ABILITY_ARMS_ALWAYSCHARGED, ARMOR_ABILITY_ARMS_MISSILE
};
enum e_ARMOR_ABILITIES_LEGS {
    ARMOR_ABILITY_LEGS_DOUBLEJUMP, ARMOR_ABILITY_LEGS_AIRDASH, ARMOR_ABILITY_LEGS_SHORYUKEN
};
enum e_ARMOR_ABILITIES_BODY {
    ARMOR_ABILITY_BODY_HALFDAMAGE, ARMOR_ABILITY_BODY_EXTENDEDIMMUNITY, ARMOR_ABILITY_BODY_SPIKESIMMMUNE, ARMOR_ABILITY_BODY_NOPUSHBACK
};

#define FS_FACE_FILENAME_MAX 20
#define FS_PLATER_ITEMS_N 2

#define SPIKES_DAMAGE 999

#define CURRENT_FILE_FORMAT format_v4


#define SCENES_LINE_H_DIFF 12
#define SCENES_TEXT_BOTTOM_POSY 160

#define VIDEO_MODE_COLORS 16

#define COLORKEY1_R 55
#define COLORKEY1_G 255
#define COLORKEY1_B 0

#define COLORKEY2_R 255
#define COLORKEY2_G 0
#define COLORKEY2_B 255

#define COLORKEY3_R 0
#define COLORKEY3_G 255
#define COLORKEY3_B 255

#define ANIM_TILE_MAX_FRAMES 10

// --- STRINGS BEGIN --- //
#define STRINGS_LINE_SIZE 60

enum E_STRINGS_INGAME {
    strings_ingame_newgame,
    strings_ingame_loadgame,
    strings_ingame_config,
    strings_ingame_about,
    strings_ingame_password,
    strings_ingame_gameover,
    strings_ingame_and,
    strings_ingame_selectgame,
    strings_ingame_savegameerror1,
    strings_ingame_savegameerror2,
    strings_ingame_savegameerror3,
    strings_ingame_copyrightline,
    strings_ingame_audio,
    strings_ingame_input,
    strings_ingame_video,
    strings_ingame_leavestage,
    strings_ingame_language,
    strings_ingame_video_windowed,
    strings_ingame_video_fullscreen,
    strings_ingame_video_noscale,
    strings_ingame_video_size2x,
    strings_ingame_video_scale2x,
    strings_ingame_config_restart1,
    strings_ingame_config_restart2,
    strings_ingame_config_restart3,
    strings_ingame_config_presstorestart,
    strings_ingame_pressanykey,
    strings_ingame_config_set,
    strings_ingame_config_key_up,
    strings_ingame_config_key_down,
    strings_ingame_config_key_left,
    strings_ingame_config_key_right,
    strings_ingame_config_key_jump,
    strings_ingame_config_key_dash,
    strings_ingame_config_key_fire,
    strings_ingame_config_key_start,
    strings_ingame_config_key_keyl,
    strings_ingame_config_key_keyr,
    strings_ingame_config_key_pressnew,
    strings_ingame_config_key_reset,
    strings_ingame_config_key_directional,
    strings_ingame_config_key_directional_analog,
    strings_ingame_config_key_directional_digital,
    strings_ingame_passwordinvalid,
    strings_ingame_pressstart,
    strings_ingame_engineerror,
    strings_ingame_nogames,
    strings_ingame_quitgame,
    strings_ingame_yes,
    strings_ingame_no,
    strings_ingame_life,
    strings_ingame_item,
    strings_ingame_gotarmor_type_ability,
    strings_ingame_gotarmor_type_arms,
    strings_ingame_gotarmor_type_arms_msg1,
    strings_ingame_gotarmor_type_arms_msg2,
    strings_ingame_gotarmor_type_arms_msg3,
    strings_ingame_gotarmor_type_arms_msg4,
    strings_ingame_gotarmor_type_legs,
    strings_ingame_gotarmor_type_legs_msg1,
    strings_ingame_gotarmor_type_legs_msg2,
    strings_ingame_gotarmor_type_legs_msg3,
    strings_ingame_gotarmor_type_legs_msg4,
    strings_ingame_gotarmor_type_body,
    strings_ingame_gotarmor_type_body_msg1,
    strings_ingame_gotarmor_type_body_msg2,
    strings_ingame_gotarmor_type_body_msg3,
    strings_ingame_gotarmor_type_body_msg4,

    strings_ingame_difficulty_select,
    strings_ingame_difficulty_easy,
    strings_ingame_difficulty_normal,
    strings_ingame_difficulty_hard,

    strings_ingame_config_input_selected_joystick,
    strings_ingame_config_input_buttons,
    strings_ingame_config_input_turbo_mode,

    strings_ingame_config_on,
    strings_ingame_config_off,
    strings_ingame_config_quitgame,
    strings_ingame_armor,
    strings_ingame_config_audio_volume_music,
    strings_ingame_config_audio_volume_sfx,
    strings_ingame_config_enabled,
    strings_ingame_config_disabled,

    strings_ingame_ready_message,

    strings_ingame_config_select_player,

    strings_ingame_config_press_start_to_select,

    strings_ingame_manual,
    strings_ingame_mode,
    strings_ingame_video_scale_mode,

    strings_config_android_hidescreencontrols,
    strings_config_android_screencontrolssize,
    strings_config_android_useplayservices,
    strings_config_wii_joysticktype,

    strings_config_android_screencontrolssize_SMALL,
    strings_config_android_screencontrolssize_MEDIUM,
    strings_config_android_screencontrolssize_BIG,
    strings_config_wii_joysticktype_WIIMOTE,
    strings_config_wii_joysticktype_CLASSIC,
    strings_config_wii_joysticktype_GAMECUBE,

    strings_config_wii_platformspecific,

    strings_config_keys_unet,
    strings_ingame_config_graphics_performance,

    strings_ingame_COUNT
};


enum E_DIFFICULTY_MODES  {
    DIFFICULTY_EASY,
    DIFFICULTY_NORMAL,
    DIFFICULTY_HARD,
    DIFFICULTY_COUNT
};

#define FS_COMMONSTRING_ID_SIZE 30
#define FS_COMMON_STRINGS_DIALOG 27
#define FS_COMMON_ITEMS_MAX 50


#define EDITOR_ANIM_PALETE_MAX_COL 8

// --- STRINGS  END --- //


#define FS_ANIM_TILE_MAX_FRAMES 6 // is we want to use more than 6 frames, we are doing it wrong...

enum E_BG_SCROLL_MODE {
    BG_SCROLL_MODE_NONE,
    BG_SCROLL_MODE_UP,
    BG_SCROLL_MODE_DOWN,
    BG_SCROLL_MODE_LEFT,
    BG_SCROLL_MODE_RIGHT,
    BG_SCROLL_MODE_DIAGONAL
};


enum e_SCREEN_GFX {
    SCREEN_GFX_NONE,
    SCREEN_GFX_RAIN,
    SCREEN_GFX_SNOW,
    SCREEN_GFX_WATER,
    SCREEN_GFX_TRAIN,
    SCREEN_GFX_FLASH,
    SCREEN_GFX_LIGHTINGBOLT,
    SCREEN_GFX_SHADOW_TOP,
    SCREEN_GFX_INFERNO,
    SCREEN_GFX_COUNT
};


enum e_SCREEN_GFX_MODE {
    SCREEN_GFX_MODE_FULLMAP,
    SCREEN_GFX_MODE_BACKGROUND,
    SCREEN_GFX_MODE_OVERLAY,
    SCREEN_GFX_MODE_COUNT
};

enum e_ANDROID_BUTTON_SIZE {
    ANDROID_BUTTON_SMALL,
    ANDROID_BUTTON_NORMAL,
    ANDROID_BUTTON_BIG,
    ANDROID_BUTTON_COUNT
};

#define PLAYER_RIGHT_TO_LEFT_DIFF 5
#define PLAYER_LEFT_TO_RIGHT_DIFF 5

#define TOP_STARS_LIMIT 65
#define BOTTOM_STARS_LIMIT 175

#define CHAR_OBJ_COLlISION_KILL_ADJUST 4

#define AUTOSCROLL_START_DELAY_FRAMES 15

#define STAGE_DIALOG_NUMBER 60

#define OBJ_JUMP_Y_ADJUST 6

enum e_LANGUAGES {
    LANGUAGE_ENGLISH,
    LANGUAGE_FRENCH,
    LANGUAGE_SPANISH,
    LANGUAGE_ITALIAN,
    LANGUAGE_PORTUGUESE,
    LANGUAGE_COUNT
};

//#define SHOW_HITBOXES 1

#define DEMO_VERSION_STAGE1 4
#define DEMO_VERSION_STAGE2 8

enum e_PERFORMANCE_MODES {
    PERFORMANCE_MODE_LOW,
    PERFORMANCE_MODE_NORMAL,
    PERFORMANCE_MODE_HIGH,
    PERFORMANCE_MODE_COUNT
};

enum STAGE_SELECT_EDIT_MODES {
    STAGE_SELECT_EDIT_MODE_LOCKED,
    STAGE_SELECT_EDIT_MODE_PATH,
    STAGE_SELECT_EDIT_MODE_STAGE,
    STAGE_SELECT_EDIT_MODE_CASTLE,
    STAGE_SELECT_EDIT_MODE_COUNT
};

#endif // DEFINES_H


