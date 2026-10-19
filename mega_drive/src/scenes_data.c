#include "scenes_data.h"
#include "resources.h"
#include "scene_par0_l0_tiles.h"
#include "scene_par0_l0_map.h"
#include "scene_par0_l1_tiles.h"
#include "scene_par0_l1_map.h"

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

// Parallax #1: 2 layers, 10ms/frame, 4000ms
static const SceneLayer scene_par0_layers[] = {
    { scene_par0_l0_tiles_data, SCENE_PAR0_L0_TILE_COUNT,
      scene_par0_l0_map, SCENE_PAR0_L0_MAP_W, SCENE_PAR0_L0_MAP_H,
      &scene_par0_l0_pal, 1, 1 },
    { scene_par0_l1_tiles_data, SCENE_PAR0_L1_TILE_COUNT,
      scene_par0_l1_map, SCENE_PAR0_L1_MAP_W, SCENE_PAR0_L1_MAP_H,
      &scene_par0_l1_pal, 38, 3 },
};
const SceneParallax scene_parallax[1] = {
    { scene_par0_layers, 2, 10, 4000, 0 },
};

const SceneClear scene_clear[1] = {
    { 20, 20, 280, 80, 0x210 }, // INTRO Lab BG EYES
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
    { "20XX A.D.", "KYOTO CITY, JAPAN.", "", "", "", "" },
    { "STORM RAGES JAPAN'S CAPITAL.", "HAVOK ROBOT FORCES ATTACK", "HUMANS, CAUSING FEAR IN OTHER", "PLACE THAT THE MACHINE'S", "REBELLION COULD SPREAD FOR", "THE ENTIRE WORLD." },
    { "IN KYOTO, SCIENTISTS AND", "SECURITY FORCES WORK TO", "PREVENT THE DANGER WITHOUT", "REST.", "", "UNTIL ONE NIGHT..." },
    { "BATTLE REACHES THE CITY", "LIKE A TSUNAMI!", "AN HORDE OF EVIL ROBOTS", "WAGE WAR AND BRING CHAOS.", "", "" },
    { "DESTRIN: THIS CAN'T BE!", "YOU ARE NOT SUPPOSED TO BE", "SO STRONG! YOU ARE A MONSTER!", "BETA: YOU SPEAK NONSENSE.", "I AM AS STRONG AS NEEDED", "TO PROTECT THE ONES YOU HURT." },
    { "BETA: I WILL END THIS NOW.", "MAKE YOUR PRAYERS AND GET", "READY TO BE DELETED.", "DESTRIN: WHAT?! NO, YOU CAN'T", "YOUR PROGRAMMING WON'T", "LET YOU HURT HUMANS!" },
    { "20XX A.D.", "KYOTO CITY, JAPAN.", "", "", "", "" },
    { "ROCK: BETA, STOP NOW!", "DON'T DO IT!", "I CAN'T LET YOU HURT HIM.", "BETA: ROCK... WHY?", "WHY DO YOU PROTECT SOMEONE", "WHO TRIED TO DESTROY OUR CITY?" },
    { "ROCK: IF YOU HURT HIM, IT CAN", "START A WAR BETWEEN ROBOTS AND", "HUMANS. I CAN'T ALLOW THAT.", "", "I AM PROTECTTING NOT HIM,", "BUT YOU, MY BROTHER." },
    { "BETA: UNDERSTOOD.", "BUT NEVER FORGET ONE THING,", "ANYONE WHO HE HURTS FROM", "NOW ON, IT WILL BE YOUR FAULT.", "ROCK: I KNOW, AND I WON'T", "LET HIM SCAPE." },
    { "ROCK: BETA...", "WHAT WILL YOU DO NOW?", "WILL YOU COME HOME?", "BETA: NO. I REFUSE TO BE A", "TOY, A SERVE. I WILL FOLLOW ", "THE WIND, AS A FREE BEING." },
    { "UNABLE TO FIGHT THE ENEMY,", "SECURITY FORCES CALL FOR HELP.", "", "A GROUP OF SCIENTISTS GET", "TOGETHER TO CREATE A DEFENSE", "SYSTEM TO PROTECT THE CITY." },
    { "A YOUNG PROFESSOR CALLED", "DR. KANOTUS, CALLS FOR THE", "CREATION OF A POLICE FORCE", "MADE OF STEEL AS THE ENEMY.", "", "" },
    { "AFTER A SUCCESSFUL FIRST", "CREATION, CALLED BETABOT,", "DR. KANOTUS IMPROVES UPON", "THE INITIAL PROJECT AND CREATE", "A SECOND ROBOT, THIS TIME", "CALLED ROCKBOT." },
    { "WITHOUT ANY TIME LEFT,", "BOTH UNEXPERIENCED AND", "UNTESTED ROBOTS MUST", "BECOME THE CITY'S SALVATION.", "", "ONWARDS, ROCK AND BETA!!" },
    { "YOU SHOULDN'T BE ABLE TO", "DEFEAT ME, THIS IS IMPOSSIBLE.", "NO ROBOT SHOULD BE SO STRONG,", "I CAN'T ACCEPT THIS!", "", "" },
    { "THIS IS WHY I WON.", "YOU LET YOURSELF GO GUIDED", "BY YOUR EGO.", "HUMANS AND MACHINES, IT MAKES", "NO DIFFERENCE. YOUR COURAGE", "IS THAT COUNTS." },
    { "BETA WANTS YOU TO BE ERASED,", "BUT I KNOW BETTER.", "YOU ARE NO THREAD,", "YOU ARE JUST... SCARED.", "", "" },
    { "THIS IS...", "A LESSON I WILL REMEMBER,", "ROCKBOT.", "NEXT TIME, I WILL BE MORE", "PREPARED.", "" },
};
