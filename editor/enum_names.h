#ifndef ENUM_NAMES_H
#define ENUM_NAMES_H

#include <vector>
#include <string>

std::vector<std::string> PROJECTILE_TRAJECTORIES_NAMES;
std::vector<std::string> AI_ACTION_NAMES;
std::vector<std::string> AI_ACTION_SHOT_OPTIONS;
std::vector<std::string> AI_ACTION_WALK_OPTIONS;
std::vector<std::string> AI_ACTION_FLY_OPTIONS;
std::vector<std::string> AI_ACTION_WALL_WALK_OPTIONS;
std::vector<std::string> AI_ACTION_JUMP_OPTIONS;
std::vector<std::string> AI_ACTION_AIR_WALK_OPTIONS;
std::vector<std::string> AI_ACTION_TELEPORT_OPTIONS;
std::vector<std::string> AI_ACTION_DASH_OPTIONS;
std::vector<std::string> AI_ACTION_GRAB_WALL_OPTIONS;
std::vector<std::string> ANIMATION_TYPE_LIST;
std::vector<std::string> AI_ACTION_SFX_OPTION_NAMES;
std::vector<std::string> AI_ACTION_SHOT_MULTIPLE_PROJECTILE_NAMES;

void init_enum_names() {
    PROJECTILE_TRAJECTORIES_NAMES.push_back("LINEAR");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("ARC");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("SINUSOID");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("CHAIN");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("QUAKE");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("BOMB");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("FOLLOW");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("PROGRESSIVE");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("FREEZE");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("DIAGONAL_UP");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("DIAGONAL_DOWN");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("CENTERED");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("ZIGZAG");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("TARGET_DIRECTION");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("ARC_TO_TARGET");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("TARGET_EXACT");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("TRAJECTORY FALL BOMB");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("LASER");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("PUSH");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("ARC SMALL");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("RING");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("LIGHTING");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("SPIRAL");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("BOUNCING");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("INVERSE LINEAR");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("DOUBLE LINEAR");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("DOUBLE DIAGONAL");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("BOMB RAIN");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("LARGE BEAM");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("PULL");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("SLASH");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("UP");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("DOWN");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("FALL AND SEEK");
    PROJECTILE_TRAJECTORIES_NAMES.push_back("TRAJECTORY AHEAD AND BACK");


    // -------------------------------------------------------------- //
    AI_ACTION_NAMES.push_back("WALK");
    AI_ACTION_NAMES.push_back("FLY");
    AI_ACTION_NAMES.push_back("JUMP");
    AI_ACTION_NAMES.push_back("WAIT UNTIL PLAYER IS IN RANGE");
    AI_ACTION_NAMES.push_back("SAVE POINT");
    AI_ACTION_NAMES.push_back("ATTACK AHEAD");
    AI_ACTION_NAMES.push_back("ATTACK PLAYER DIRECTION");
    AI_ACTION_NAMES.push_back("ATTACK (INVERT DIRECTION)");
    AI_ACTION_NAMES.push_back("AIR WALK");
    AI_ACTION_NAMES.push_back("FALL TO GROUND");
    AI_ACTION_NAMES.push_back("TELEPORT");
    AI_ACTION_NAMES.push_back("DASH");
    AI_ACTION_NAMES.push_back("GRAB WALL");
    AI_ACTION_NAMES.push_back("SPAWN NPC");
    AI_ACTION_NAMES.push_back("CHANGE ANIMATION TYPE");
    AI_ACTION_NAMES.push_back("REPLACE ITSELF");
    AI_ACTION_NAMES.push_back("CIRCLE PLAYER");
    AI_ACTION_NAMES.push_back("CHANGE ANIMATION TYPE [REVERSE]");
    AI_ACTION_NAMES.push_back("JUMP ATTACK [UP]");
    AI_ACTION_NAMES.push_back("JUMP ATTACK [AHEAD]");
    AI_ACTION_NAMES.push_back("WAIT TIME [RANDOM]");
    AI_ACTION_NAMES.push_back("MORPH INTO");
    AI_ACTION_NAMES.push_back("PLAY SOUND EFFECT");
    AI_ACTION_NAMES.push_back("WALL-WALK");
    AI_ACTION_NAMES.push_back("WALL-WALK-SHOOT");
    AI_ACTION_NAMES.push_back("MULTIPLE ATTACK");
    AI_ACTION_NAMES.push_back("EXPLODE ITSELF");
    AI_ACTION_NAMES.push_back("THROW ITEM");

    // -------------------------------------------------------------- //
    AI_ACTION_SHOT_OPTIONS.push_back("FACE PLAYER");
    AI_ACTION_SHOT_OPTIONS.push_back("AHEAD");

    // -------------------------------------------------------------- //
    AI_ACTION_WALK_OPTIONS.push_back("HORIZONTAL AHEAD");
    AI_ACTION_WALK_OPTIONS.push_back("HORIZONTAL TURN");
    AI_ACTION_WALK_OPTIONS.push_back("TO PLAYER");
    AI_ACTION_WALK_OPTIONS.push_back("TO SAVED POINT");
    AI_ACTION_WALK_OPTIONS.push_back("TURN TO PLAYER");
    AI_ACTION_WALK_OPTIONS.push_back("RANDOM DIRECTION");
    AI_ACTION_WALK_OPTIONS.push_back("TO OPPOSITE WALL");
    AI_ACTION_WALK_OPTIONS.push_back("AHEAD FALLING");

    // -------------------------------------------------------------- //
    AI_ACTION_FLY_OPTIONS.push_back("AHEAD");
    AI_ACTION_FLY_OPTIONS.push_back("TURN");
    AI_ACTION_FLY_OPTIONS.push_back("UP");
    AI_ACTION_FLY_OPTIONS.push_back("DOWN");
    AI_ACTION_FLY_OPTIONS.push_back("TO PLAYER");
    AI_ACTION_FLY_OPTIONS.push_back("TO SAVED POINT");
    AI_ACTION_FLY_OPTIONS.push_back("TO RANDOM POINT");
    AI_ACTION_FLY_OPTIONS.push_back("FALL");
    AI_ACTION_FLY_OPTIONS.push_back("DRILL DOWN");
    AI_ACTION_FLY_OPTIONS.push_back("VERTICAL CENTER");
    AI_ACTION_FLY_OPTIONS.push_back("PLAYER DIRECTION");
    AI_ACTION_FLY_OPTIONS.push_back("TO OPPOSITE WALL");
    AI_ACTION_FLY_OPTIONS.push_back("TO OPPOSITE (SHOOT #1)");
    AI_ACTION_FLY_OPTIONS.push_back("TO PLAYER X");
    AI_ACTION_FLY_OPTIONS.push_back("TO PLAYER Y");
    AI_ACTION_FLY_OPTIONS.push_back("AHEAD [ZIG-ZAG-Y]");
    AI_ACTION_FLY_OPTIONS.push_back("RANDOM X");
    AI_ACTION_FLY_OPTIONS.push_back("RANDOM Y");
    AI_ACTION_FLY_OPTIONS.push_back("AHEAD [SIN-Y]");
    AI_ACTION_FLY_OPTIONS.push_back("AIR-DASH TO PLAYER");
    AI_ACTION_FLY_OPTIONS.push_back("DOWN RANDOM DIAGONAL");
    AI_ACTION_FLY_OPTIONS.push_back("UP UNTIL GROUND");
    AI_ACTION_FLY_OPTIONS.push_back("DOWN INTO GROUND");
    AI_ACTION_FLY_OPTIONS.push_back("DIAGONMAL UP");
    AI_ACTION_FLY_OPTIONS.push_back("DIAGONMAL DOWN");

    // -------------------------------------------------------------- //

    AI_ACTION_WALL_WALK_OPTIONS.push_back("LEFT");
    AI_ACTION_WALL_WALK_OPTIONS.push_back("RIGHT");
    AI_ACTION_WALL_WALK_OPTIONS.push_back("UP");
    AI_ACTION_WALL_WALK_OPTIONS.push_back("DOWN");

    // -------------------------------------------------------------- //

    AI_ACTION_JUMP_OPTIONS.push_back("AHEAD");
    AI_ACTION_JUMP_OPTIONS.push_back("TO RANDOM POINT");
    AI_ACTION_JUMP_OPTIONS.push_back("UP");
    AI_ACTION_JUMP_OPTIONS.push_back("TO PLAYER");
    AI_ACTION_JUMP_OPTIONS.push_back("TO ROOT");
    AI_ACTION_JUMP_OPTIONS.push_back("TO SAVED POINT");
    AI_ACTION_JUMP_OPTIONS.push_back("TO PLAYER DIRECTION");
    AI_ACTION_JUMP_OPTIONS.push_back("TELEPORT IN");
    AI_ACTION_JUMP_OPTIONS.push_back("TELEPORT OUT");
    AI_ACTION_JUMP_OPTIONS.push_back("LONG AHEAD");
    // -------------------------------------------------------------- //
    AI_ACTION_AIR_WALK_OPTIONS.push_back("LEFT");
    AI_ACTION_AIR_WALK_OPTIONS.push_back("RIGHT");
    AI_ACTION_AIR_WALK_OPTIONS.push_back("TO PLAYER");
    // -------------------------------------------------------------- //
    AI_ACTION_TELEPORT_OPTIONS.push_back("LEFT");
    AI_ACTION_TELEPORT_OPTIONS.push_back("RIGHT");
    AI_ACTION_TELEPORT_OPTIONS.push_back("TO PLAYER");
    AI_ACTION_TELEPORT_OPTIONS.push_back("RANDOM X");
    AI_ACTION_TELEPORT_OPTIONS.push_back("RANDOM Y");
    AI_ACTION_TELEPORT_OPTIONS.push_back("RANDOM POINT (X&Y)");
    AI_ACTION_TELEPORT_OPTIONS.push_back("AHEAD");
    // -------------------------------------------------------------- //
    AI_ACTION_DASH_OPTIONS.push_back("LEFT");
    AI_ACTION_DASH_OPTIONS.push_back("RIGHT");
    AI_ACTION_DASH_OPTIONS.push_back("TO PLAYER");
    AI_ACTION_DASH_OPTIONS.push_back("TO OPPOSITE WALL");
    AI_ACTION_DASH_OPTIONS.push_back("AHEAD");
    AI_ACTION_DASH_OPTIONS.push_back("TO NEAR PLAYER");
    // -------------------------------------------------------------- //
    AI_ACTION_GRAB_WALL_OPTIONS.push_back("LEFT");
    AI_ACTION_GRAB_WALL_OPTIONS.push_back("RIGHT");
    // -------------------------------------------------------------- //

    ANIMATION_TYPE_LIST.push_back("STAND");
    ANIMATION_TYPE_LIST.push_back("WALK");
    ANIMATION_TYPE_LIST.push_back("JUMP");
    ANIMATION_TYPE_LIST.push_back("ATTACK");
    ANIMATION_TYPE_LIST.push_back("TELEPORT");
    ANIMATION_TYPE_LIST.push_back("JUMP ATTACK");
    ANIMATION_TYPE_LIST.push_back("WALK ATTACK");
    ANIMATION_TYPE_LIST.push_back("HIT");
    ANIMATION_TYPE_LIST.push_back("STAIRS");
    ANIMATION_TYPE_LIST.push_back("STAIRS SEMI");
    ANIMATION_TYPE_LIST.push_back("STAIRS ATTACK");
    ANIMATION_TYPE_LIST.push_back("STAIRS MOVE");
    ANIMATION_TYPE_LIST.push_back("THROW");
    ANIMATION_TYPE_LIST.push_back("TURN [HORIZONTAL]");
    ANIMATION_TYPE_LIST.push_back("MOVE SEMI");
    ANIMATION_TYPE_LIST.push_back("FLY/WALK-AIR");
    ANIMATION_TYPE_LIST.push_back("INTRO-PRESENTS");
    ANIMATION_TYPE_LIST.push_back("SPECIAL ATTACK");
    ANIMATION_TYPE_LIST.push_back("SLIDE/DASH");
    ANIMATION_TYPE_LIST.push_back("SHIELD");
    ANIMATION_TYPE_LIST.push_back("TURN [VERTICAL]");
    ANIMATION_TYPE_LIST.push_back("ATTACK THROW");
    ANIMATION_TYPE_LIST.push_back("ATTACK [DIAGONAL UP]");
    ANIMATION_TYPE_LIST.push_back("ATTACK [DIAGONAL DOWN]");
    ANIMATION_TYPE_LIST.push_back("ATTACK [UP]");
    ANIMATION_TYPE_LIST.push_back("ATTACK [DOWN]");
    ANIMATION_TYPE_LIST.push_back("ATTACK [SPACIAL]");
    ANIMATION_TYPE_LIST.push_back("LOOK UP");
    ANIMATION_TYPE_LIST.push_back("GRAB WALL");
    ANIMATION_TYPE_LIST.push_back("FLY");

    // -------------------------------------------------------------- //
    AI_ACTION_SFX_OPTION_NAMES.push_back("DINOSSAUR GROWL");

    // -------------------------------------------------------------- //
    AI_ACTION_SHOT_MULTIPLE_PROJECTILE_NAMES.push_back("5 UP");
    AI_ACTION_SHOT_MULTIPLE_PROJECTILE_NAMES.push_back("5 DOWN");
    AI_ACTION_SHOT_MULTIPLE_PROJECTILE_NAMES.push_back("5 AHEAD");
    AI_ACTION_SHOT_MULTIPLE_PROJECTILE_NAMES.push_back("3 UP");
    AI_ACTION_SHOT_MULTIPLE_PROJECTILE_NAMES.push_back("3 DOWN");
    AI_ACTION_SHOT_MULTIPLE_PROJECTILE_NAMES.push_back("3 AHEAD");
    AI_ACTION_SHOT_MULTIPLE_PROJECTILE_NAMES.push_back("8");

}






#endif // ENUM_NAMES_H
