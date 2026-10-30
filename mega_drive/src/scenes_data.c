#include "scenes_data.h"
#include "resources.h"
#include "music.h"
#include "scene_par0_l0_tiles.h"
#include "scene_par0_l0_map.h"
#include "scene_par0_l1_tiles.h"
#include "scene_par0_l1_map.h"
#include "scene_img6_tiles.h"
#include "scene_img6_map.h"
#include "scene_img8_tiles.h"
#include "scene_img8_map.h"
#include "scene_img9_tiles.h"
#include "scene_img9_map.h"
#include "scene_img10_tiles.h"
#include "scene_img10_map.h"
#include "scene_img11_tiles.h"
#include "scene_img11_map.h"
#include "scene_img12_tiles.h"
#include "scene_img12_map.h"

// Opening Unbeaten #1
static const SceneOp scene0_ops[] = {
    { 0, 0, 100 }, // CLEAR_SCREEN
    { 8, 0, 2200 }, // SHOW_TEXT
    { 0, 0, 100 }, // CLEAR_SCREEN
    { 6, 0, 100 }, // PLAY_MUSIC
    { 10, 0, 100 }, // PARALLAX
    { 0, 0, 100 }, // CLEAR_SCREEN
    { 8, 1, 4000 }, // SHOW_TEXT
    { 8, 2, 4000 }, // SHOW_TEXT
    { 8, 3, 4000 }, // SHOW_TEXT
    { 8, 4, 4000 }, // SHOW_TEXT
};

// Opening Unbeaten LAB [#2]
static const SceneOp scene1_ops[] = {
    { 0, 0, 0 }, // CLEAR_SCREEN
    { 2, 1, 0 }, // MOVE_IMAGE
    { 2, 2, 1000 }, // MOVE_IMAGE
    { 8, 12, 0 }, // SHOW_TEXT
    { 2, 3, 1000 }, // MOVE_IMAGE
    { 8, 13, 0 }, // SHOW_TEXT
    { 0, 0, 0 }, // CLEAR_SCREEN
    { 1, 0, 0 }, // CLEAR_AREA
    { 2, 6, 0 }, // MOVE_IMAGE
    { 2, 7, 0 }, // MOVE_IMAGE
    { 4, 4, 1000 }, // SHOW_ANIMATION
    { 8, 14, 3000 }, // SHOW_TEXT
};

// Ending Rockbot
static const SceneOp scene2_ops[] = {
    { 0, 0, 0 }, // CLEAR_SCREEN
    { 6, 2, 0 }, // PLAY_MUSIC
    { 1, 0, 0 }, // CLEAR_AREA
    { 2, 8, 0 }, // MOVE_IMAGE
    { 8, 15, 3200 }, // SHOW_TEXT
    { 2, 10, 0 }, // MOVE_IMAGE
    { 8, 16, 3200 }, // SHOW_TEXT
    { 8, 17, 3200 }, // SHOW_TEXT
    { 8, 18, 6000 }, // SHOW_TEXT
};

// Ending Betabot
static const SceneOp scene3_ops[] = {
    { 0, 0, 0 }, // CLEAR_SCREEN
    { 6, 2, 0 }, // PLAY_MUSIC
    { 1, 0, 0 }, // CLEAR_AREA
    { 2, 6, 0 }, // MOVE_IMAGE
    { 2, 8, 0 }, // MOVE_IMAGE
    { 8, 11, 3200 }, // SHOW_TEXT
    { 8, 5, 3200 }, // SHOW_TEXT
    { 8, 6, 3200 }, // SHOW_TEXT
    { 1, 0, 0 }, // CLEAR_AREA
    { 2, 6, 0 }, // MOVE_IMAGE
    { 2, 9, 0 }, // MOVE_IMAGE
    { 8, 7, 3200 }, // SHOW_TEXT
    { 8, 8, 3200 }, // SHOW_TEXT
    { 8, 9, 3200 }, // SHOW_TEXT
    { 8, 10, 6000 }, // SHOW_TEXT
};

// Ending Candybot
static const SceneOp scene4_ops[] = {
    { 0, 0, 0 }, // CLEAR_SCREEN
    { 6, 2, 0 }, // PLAY_MUSIC
    { 1, 0, 0 }, // CLEAR_AREA
    { 2, 11, 0 }, // MOVE_IMAGE
    { 2, 8, 0 }, // MOVE_IMAGE
    { 8, 12, 6000 }, // SHOW_TEXT
};

// Ending KittyBot
static const SceneOp scene5_ops[] = {
    { 0, 0, 0 }, // CLEAR_SCREEN
    { 6, 2, 0 }, // PLAY_MUSIC
    { 1, 0, 0 }, // CLEAR_AREA
    { 2, 12, 0 }, // MOVE_IMAGE
    { 2, 8, 0 }, // MOVE_IMAGE
    { 8, 13, 6000 }, // SHOW_TEXT
};

const SceneDef scene_defs[6] = {
    { scene0_ops, 10 }, // Opening Unbeaten #1
    { scene1_ops, 12 }, // Opening Unbeaten LAB [#2]
    { scene2_ops, 9 }, // Ending Rockbot
    { scene3_ops, 15 }, // Ending Betabot
    { scene4_ops, 6 }, // Ending Candybot
    { scene5_ops, 6 }, // Ending KittyBot
};

// PLAY_MUSIC op seek -> MUSIC_* id (music.c owns id -> resource).
const u8 scene_music_ids[3] = {
    MUSIC_INTRO_SCENE, // INTRO [Bg-Music] = intro_scene.mod
    MUSIC_STAGE2, // Musiquinha = daisie.mod
    MUSIC_END_ANIMATION, // Ending Dialog Song = rockbot_end_animation.mod
};

// Parallax #1: 2 layers, 10ms/frame, 4000ms
static const SceneLayer scene_par0_layers[] = {
    { scene_par0_l0_tiles_data, SCENE_PAR0_L0_TILE_COUNT,
      scene_par0_l0_map, SCENE_PAR0_L0_MAP_W, SCENE_PAR0_L0_MAP_H,
      &scene_par0_l0_pal, 1, 1, 256 },
    { scene_par0_l1_tiles_data, SCENE_PAR0_L1_TILE_COUNT,
      scene_par0_l1_map, SCENE_PAR0_L1_MAP_W, SCENE_PAR0_L1_MAP_H,
      &scene_par0_l1_pal, 38, 3, 512 },
};
const SceneParallax scene_parallax[1] = {
    { scene_par0_layers, 2, 10, 4000, 0 },
};

const SceneClear scene_clear[1] = {
    { 20, 20, 280, 80, 0x210 }, // INTRO Lab BG EYES
};

const SceneImage scene_images[13] = {
    { 0, 0, -64, 0, 0, 0, 0, 0, 60, NULL, 0, NULL, 0, 0, NULL },
    { 10, 10, 10, 10, 0, 0, 0, 0, 100, NULL, 0, NULL, 0, 0, NULL },
    { 214, 82, 214, 82, 0, 0, 0, 0, 100, NULL, 0, NULL, 0, 0, NULL },
    { 214, 82, 214, 82, 0, 0, 0, 0, 100, NULL, 0, NULL, 0, 0, NULL },
    { -188, 10, 117, 10, 0, 0, 0, 0, 5, NULL, 0, NULL, 0, 0, NULL },
    { 0, 0, 124, 86, 0, 0, 0, 0, 100, NULL, 0, NULL, 0, 0, NULL },
    { 40, 33, 40, 33, 0, 0, 0, 0, 100, scene_img6_tiles_data, SCENE_IMG6_TILE_COUNT, scene_img6_map, SCENE_IMG6_MAP_W, SCENE_IMG6_MAP_H, &scene_img6_pal },
    { 220, 33, 220, 33, 0, 0, 0, 0, 100, NULL, 0, NULL, 0, 0, NULL },
    { 220, 33, 220, 33, 0, 0, 0, 0, 100, scene_img8_tiles_data, SCENE_IMG8_TILE_COUNT, scene_img8_map, SCENE_IMG8_MAP_W, SCENE_IMG8_MAP_H, &scene_img8_pal },
    { 220, 33, 220, 33, 0, 0, 0, 0, 100, scene_img9_tiles_data, SCENE_IMG9_TILE_COUNT, scene_img9_map, SCENE_IMG9_MAP_W, SCENE_IMG9_MAP_H, &scene_img9_pal },
    { 40, 33, 40, 33, 0, 0, 0, 0, 100, scene_img10_tiles_data, SCENE_IMG10_TILE_COUNT, scene_img10_map, SCENE_IMG10_MAP_W, SCENE_IMG10_MAP_H, &scene_img10_pal },
    { 40, 33, 40, 33, 0, 0, 0, 0, 100, scene_img11_tiles_data, SCENE_IMG11_TILE_COUNT, scene_img11_map, SCENE_IMG11_MAP_W, SCENE_IMG11_MAP_H, &scene_img11_pal },
    { 40, 33, 40, 33, 0, 0, 0, 0, 100, scene_img12_tiles_data, SCENE_IMG12_TILE_COUNT, scene_img12_map, SCENE_IMG12_MAP_W, SCENE_IMG12_MAP_H, &scene_img12_pal },
};

const SceneTextPos scene_text_pos[SCENE_TEXT_RECORDS] = {
    { 0, 0, 2 }, // INTRO [20XX]
    { 0, 0, 2 }, // INTRO [#1]
    { 0, 0, 2 }, // INTRO [#2]
    { 0, 0, 2 }, // INTRO [#3]
    { 0, 0, 2 }, // INTRO #4
    { 0, 0, 0 }, // ENDING BETABOT #2
    { 0, 0, 0 }, // ENDING BETABOT #3
    { 0, 0, 0 }, // ENDING BETABOT #4
    { 0, 0, 0 }, // ENDING BETABOT #5
    { 0, 0, 0 }, // ENDING BETABOT #6
    { 0, 0, 0 }, // ENDING BETABOT #7
    { 0, 0, 0 }, // ENDING BETABOT #1
    { 0, 0, 0 }, // ENDING CANDY #1
    { 0, 0, 0 }, // ENDING KITTY #1
    { 0, 0, 0 }, // INTRO #7
    { 0, 0, 0 }, // ENDING ROCKBOT #1
    { 0, 0, 0 }, // ENDING ROCKBOT #2
    { 0, 0, 0 }, // ENDING ROCKBOT #3
    { 0, 0, 0 }, // ENDING ROCKBOT #4
};

const char *const scene_text[SCENE_TEXT_RECORDS][SCENE_TEXT_LINES] = {
    { "IN THE DISTANT FUTURE...", "RIO GRANDE DO SUL, BRAZIL", "", "", "", "" },
    { "GENETIC ENGEENERING MERGED", "WITH ROBOTICS TO CREATE A NEW", "RACE CALLED \"BOTS\".", "BESIDES THE ETERNAL YOUTH", "APPEARANCE, THEY ARE VERY", "STRONG AND RESISTANT." },
    { "HOWEVER, A SCIENTIST CALLED", "DESTRIN, HACKED THE \"BOTS\"", "EMBRYO SYSTEM, AND PLANTED", "IN SOME OF THEM A MALWARE", "CODE THAT ALLOWS HIM TO", "DISTORT THEIR MINDS." },
    { "IN A STORMY NIGHT,", "IN THE CAPITAL OF THE", "RIO-GRANDE DO SUL REPUBLIC,", "EXPLOSIONS HIT THE ENERGY", "FACTORY THAT PROVIDE THE FUEL", "\"BOTS\" USE AS POWER." },
    { "COULD IT BE THAT DESTRIN IS", "TRYING TO OBTAIN THE RAW", "MATERIALS NEEDED TO BUILD A", "EMBRYONIC CHAMBER OF HIS", "OWN AND CREATE A FULL", "ARMY OF HAVOK \"BOTS\"!?" },
    { "BETA: I WILL END THIS NOW.", "MAKE YOUR PRAYERS AND GET", "READY TO BE DELETED.", "DESTRIN: WHAT?! NO, YOU CAN'T", "YOUR GENES WON'T LET", "YOU TO HURT HUMANS!" },
    { "YOU ARE STRONGLY MISTAKEN", "IF YOU DO BELIEVE I AM A", "MERE \"CODE\" RUNNING ON ", "A FLESH AND METAL SHELL.", "", "NOW, BEGONE FOREVER!" },
    { "ROCK: BETA, STOP NOW!", "DON'T DO IT!", "I CAN'T LET YOU HURT HIM.", "BETA: ROCK... WHY?", "WHY DO YOU PROTECT SOMEONE", "WHO TRIED TO DESTROY OUR CITY?" },
    { "ROCK: IF YOU HURT HIM, IT CAN", "START A WAR BETWEEN US AND", "HUMANS. I CAN'T ALLOW THAT.", "", "I AM PROTECTING NOT HIM,", "BUT YOU, MY BROTHER." },
    { "BETA: UNDERSTOOD.", "BUT NEVER FORGET ONE THING,", "ANYONE WHO HE HURTS FROM", "NOW ON, IT WILL BE YOUR FAULT.", "ROCK: I KNOW, AND I WON'T", "LET HIM SCAPE." },
    { "ROCK: BETA...", "WHAT WILL YOU DO NOW?", "WILL YOU COME HOME?", "BETA: NO. I REFUSE TO BE A", "TOY, A SERVE. I WILL FOLLOW ", "THE WIND, AS A FREE BEING." },
    { "BETA: I WILL END THIS NOW.", "MAKE YOUR PRAYERS AND GET", "READY TO BE DELETED.", "DESTRIN: WHAT?! NO, YOU CAN'T", "YOUR GENES DO NOT ALLOW", "YOU HURT HUMANS!" },
    { "CANDY: NOW BIG MEANINE,", "WILL YOU BE A GOOD BOY?", "DESTRIN: THIS CHILDISH WAY", "OF SPEAKING MAKES ME ANGRY!", "CANDY: I WILL PUT YOU IN A ", "PLACE YOU CAN COOL YOUR HEAD." },
    { "WELL DOCTOR, WE MEET", "AGAIN.", "WHAT? I KNOW NOTHING OF", "YOU, STRAY-BOT!", "WHAT? I WILL MAKE SURE YOU", "REMEMVER EVERYTHING..." },
    { "WITHOUT ANY TIME LEFT,", "BOTH UNEXPERIENCED AND", "UNTESTED ROBOTS MUST", "BECOME THE CITY'S SALVATION.", "", "ONWARDS, ROCK AND BETA!!" },
    { "YOU SHOULDN'T BE ABLE TO", "DEFEAT ME, THIS IS IMPOSSIBLE.", "NO ROBOT SHOULD BE SO STRONG,", "I CAN'T ACCEPT THIS!", "", "" },
    { "THIS IS WHY I WON.", "YOU LET YOURSELF GO GUIDED", "BY YOUR EGO.", "HUMANS AND MACHINES, IT MAKES", "NO DIFFERENCE. YOUR COURAGE", "IS THAT COUNTS." },
    { "BETA WANTS YOU TO BE ERASED,", "BUT I KNOW BETTER.", "YOU ARE NO THREAD,", "YOU ARE JUST... SCARED.", "", "" },
    { "THIS IS...", "A LESSON I WILL REMEMBER,", "ROCKBOT.", "NEXT TIME, I WILL BE MORE", "PREPARED.", "" },
};
