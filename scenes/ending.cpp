#include <cstdlib>
#include "ending.h"

extern std::string FILEPATH;

#include "soundlib.h"
extern soundLib soundManager;

extern graphicsLib graphLib;

#include "inputlib.h"
extern inputLib input;

#include "soundlib.h"
extern soundLib soundManager;

#include "timerlib.h"
extern timerLib timer;

#include "game.h"
extern game gameControl;
extern CURRENT_FILE_FORMAT::st_save game_save;


//#include "graphic/option_picker.h"
#include "dialogs.h"

#define TIMED_DIALOG_DURATION 4000

extern CURRENT_FILE_FORMAT::st_game_config game_config;
extern CURRENT_FILE_FORMAT::file_io fio;

extern CURRENT_FILE_FORMAT::file_stage stage_data;

ending::ending()
{
	// preload train
	graphLib.surfaceFromFile(std::string(FILEPATH+"data/images/backgrounds/ending_layer_train.png"), &_train_surface);
	graphLib.surfaceFromFile(std::string(FILEPATH+"data/images/backgrounds/ending_layer_background.png"),& _bg_surface);
	graphLib.surfaceFromFile(std::string(FILEPATH+"data/images/backgrounds/ending_layer_fence.png"), &_fence_surface);
	graphLib.surfaceFromFile(std::string(FILEPATH+"data/images/backgrounds/ending_layer_bushes.png"), &_bushes_surface);
}

/// @TODO: player position is wrong for some reason (must reset scrolling?)
void ending::start()
{
    boss();
    gameControl.reset_scroll();
    train();

    if (game_save.selected_player == PLAYER_ROCKBOT) {
        lab();
        look_sky_rockbot();
    } else if (game_save.selected_player == PLAYER_BETABOT) {
        look_sky_betabot();
    }
    bosses();
	credits();
}

void ending::boss()
{
	/// @TODO - get boss position from gameControl
	graphicsLib_gSurface last_stage_bg;
	graphicsLib_gSurface destrin_sprites;

    gameControl.draw_explosion(_boss_pos.x, _boss_pos.y, true);

	graphLib.initSurface(st_size(graphLib.gameScreen.width, graphLib.gameScreen.height), &last_stage_bg);
	gameControl.remove_all_projectiles();
	gameControl.show_map();
    draw_lib.update_screen();
    //graphLib.copyArea(st_position(0, 0), &graphLib.gameScreen, &last_stage_bg);
    graphLib.copy_gamescreen_area(st_rectangle(0, 0, RES_W, RES_H), st_position(0, 0), &last_stage_bg);


    st_position destrin_pos(250, (RES_H-(TILESIZE*3)-destrin_sprites.height));

	graphLib.surfaceFromFile(std::string(FILEPATH+"data/images/sprites/destrin.png"), &destrin_sprites);
	graphLib.showSurface(&last_stage_bg);
	st_position p_pos = gameControl.get_player_position(0);
    //graphLib.showSurfaceRegionAt(&destrin_sprites, st_rectangle(44, 0, 20, destrin_sprites.height), destrin_pos);
    draw_lib.update_screen();

    int boss_y = _boss_pos.y;

	// fall to ground
    gameControl.get_player(0)->reset_jump();
    gameControl.get_player(0)->cancel_slide();
    bool must_play_jump_sfx = false; // will be set only if player needs to fall
    while (gameControl.get_player(0)->hit_ground() == false) {
        //std::cout << ">> waiting for p1 to fall in the ground - p.x: " << gameControl.get_player(0)->getPosition().x << std::endl;
        gameControl.get_player(0)->gravity(false);
        graphLib.showSurface(&last_stage_bg);
        gameControl.show_player(0);

        // move destrin up
        boss_y -= 3;
        graphLib.showSurfaceRegionAt(&destrin_sprites, st_rectangle(0, 0, 26, destrin_sprites.height), st_position(_boss_pos.x, boss_y));

        draw_lib.update_screen();
        must_play_jump_sfx = true;
        timer.delay(5);
    }
    if (must_play_jump_sfx == true) {
        soundManager.play_sfx(SFX_PLAYER_JUMP);
    }



    // move remaining destrin up
    while (boss_y > -(TILESIZE*2)) {
        gameControl.get_player(0)->gravity(false);
        graphLib.showSurface(&last_stage_bg);
        gameControl.show_player(0);
        // move destrin up
        boss_y -= 3;
        graphLib.showSurfaceRegionAt(&destrin_sprites, st_rectangle(0, 0, 26, destrin_sprites.height), st_position(_boss_pos.x, boss_y));
        draw_lib.update_screen();
        timer.delay(5);
    }

    timer.delay(1000);

    // make destrin fall
    int ground_point_y = gameControl.get_player(0)->getPosition().y + gameControl.get_player(0)->get_size().height;
    //std::cout << "ground_point_y: " << ground_point_y << std::endl;
    destrin_pos.y = ground_point_y - destrin_sprites.height/2;
    must_play_jump_sfx = false; // will be set only if destrin needs to fall
    while (boss_y < destrin_pos.y) {
        //std::cout << "destrin_pos.y: " << destrin_pos.y << ", boss_y: " << boss_y << std::endl;
        gameControl.get_player(0)->gravity(false);
        graphLib.showSurface(&last_stage_bg);
        gameControl.show_player(0);
        // move destrin up
        if (boss_y+3 > destrin_pos.y) {
            boss_y ++;
        } else {
            boss_y += 3;
        }
        graphLib.showSurfaceRegionAt(&destrin_sprites, st_rectangle(0, 0, 26, destrin_sprites.height), st_position(destrin_pos.x, boss_y));
        draw_lib.update_screen();
        must_play_jump_sfx = true;
        timer.delay(5);
    }
    if (must_play_jump_sfx == true) {
        soundManager.play_sfx(SFX_PLAYER_JUMP);
    }
    graphLib.showSurface(&last_stage_bg);
    gameControl.show_player(0);
    graphLib.showSurfaceRegionAt(&destrin_sprites, st_rectangle(117, 0, 30, destrin_sprites.height), destrin_pos);
    draw_lib.update_screen();
    //std::cout << ">> ending::boss - hit ground. py: " << gameControl.get_player(0)->getPosition().y << std::endl;
    timer.delay(1000);

	gameControl.set_player_anim_type(ANIM_TYPE_WALK, 0);

	gameControl.set_player_anim_type(ANIM_TYPE_WALK, 1);

    gameControl.set_player_position(st_position(RES_W/3+100, TILESIZE), 1); // position for rockbot

	gameControl.set_player_direction(ANIM_DIRECTION_LEFT, 1);

    st_position map_scroll = gameControl.get_current_stage_scroll();

    p_pos.y = gameControl.get_player_position(0).y;



    //std::cout << ">> p_pos.x: " << p_pos.x << ", map_scroll.x: " << map_scroll.x << std::endl;

    if (p_pos.x > map_scroll.x+RES_W/3) {
		gameControl.set_player_direction(ANIM_DIRECTION_LEFT, 0);
        while (p_pos.x > map_scroll.x+RES_W/3) {
			p_pos.x -= 3;
			gameControl.set_player_position(p_pos, 0);
			graphLib.showSurface(&last_stage_bg);
			gameControl.show_player(0);
            graphLib.showSurfaceRegionAt(&destrin_sprites, st_rectangle(86, 0, 30, destrin_sprites.height), destrin_pos);
            draw_lib.update_screen();
			input.waitTime(20);
		}
	} else {
		gameControl.set_player_direction(ANIM_DIRECTION_RIGHT, 0);
        while (p_pos.x < map_scroll.x+RES_W/3) {
			p_pos.x += 3;
			gameControl.set_player_position(p_pos, 0);
			graphLib.showSurface(&last_stage_bg);
			gameControl.show_player(0);
            graphLib.showSurfaceRegionAt(&destrin_sprites, st_rectangle(86, 0, 30, destrin_sprites.height), destrin_pos);
            draw_lib.update_screen();
			input.waitTime(20);
		}
	}


    gameControl.set_player_direction(ANIM_DIRECTION_RIGHT, 0);
	gameControl.set_player_anim_type(ANIM_TYPE_STAND, 0);
	graphLib.showSurface(&last_stage_bg);
	gameControl.show_player(0);
    st_rectangle destrin_frame(86, 0, 30, destrin_sprites.height);
    if (game_save.selected_player == PLAYER_ROCKBOT) {
        destrin_frame.x = 44;
        destrin_frame.w = 20;
    }
    graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
    draw_lib.update_screen();
	input.waitTime(20);

    std::cout << "ENDING::BOSS - game_save.selected_player: " << (int)game_save.selected_player << std::endl;

    if (game_save.selected_player == PLAYER_ROCKBOT) {
        boss_rockbot(last_stage_bg, destrin_sprites, destrin_pos, p_pos);
    } else if (game_save.selected_player == PLAYER_BETABOT) {
        boss_betabot(last_stage_bg, destrin_sprites, destrin_pos, p_pos);
    } else if (game_save.selected_player == PLAYER_KITTYBOT) {
        boss_kittybot(last_stage_bg, destrin_sprites, destrin_pos, p_pos);
    } else if (game_save.selected_player == PLAYER_CANDYBOT) {
        boss_candybot(last_stage_bg, destrin_sprites, destrin_pos, p_pos);
    }

}

void ending::boss_rockbot(graphicsLib_gSurface &last_stage_bg, graphicsLib_gSurface &destrin_sprites, st_position &destrin_pos, st_position &p_pos)
{
    std::string lines[3];
    dialogs dialogs_obj;
    st_rectangle destrin_frame(44, 20, 30, destrin_sprites.height);

    gameControl.set_player_position(st_position(RES_W/3+TILESIZE*3, -TILESIZE*3), 1);
    lines[0] = "I CAN'T BELIEVE IT!!";
    lines[1] = "YOU SHOULD NOT BE ABLE";
    lines[2] = "TO DEFEAT ME.";
    dialogs_obj.show_timed_dialog("destrin.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "YOUR EGO IS YOUR.";
    lines[1] = "WEAKNESS, AND YOUR";
    lines[2] = "PLACE IS PRISION.";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);


    // betabot teleports in
    gameControl.get_player(1)->reset_jump();
    gameControl.get_player(1)->set_map(gameControl.get_player(0)->get_map());
    gameControl.get_player(1)->set_position(st_position(gameControl.get_player(0)->getPosition().x+100, -10));
    gameControl.get_player(1)->set_animation_type(ANIM_TYPE_TELEPORT);
    gameControl.get_player(1)->set_teleport_minimal_y(gameControl.get_player(0)->getPosition().y-TILESIZE);

    //std::cout << ">> p1->hit_ground: " << gameControl.get_player(1)->hit_ground() << std::endl;
    while (true) {
        //std::cout << ">> wait for p1 to hit ground <<" << std::endl;
        gameControl.get_player(1)->gravity(false);
        graphLib.showSurface(&last_stage_bg);
        graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
        gameControl.show_player(0);
        gameControl.show_player(1);
        draw_lib.update_screen();
        input.waitTime(5);
        if (gameControl.get_player(1)->getPosition().y >= gameControl.get_player(0)->getPosition().y-TILESIZE && gameControl.get_player(1)->hit_ground() == true) {
            soundManager.play_sfx(SFX_PLAYER_JUMP);
            break;
        }
    }

    gameControl.set_player_anim_type(ANIM_TYPE_STAND, 1);
    graphLib.showSurface(&last_stage_bg);
    gameControl.show_player(1);
    gameControl.show_player(0);
    graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
    draw_lib.update_screen();
    input.waitTime(20);

    lines[0] = "DESTROY HIM, ROCK,";
    lines[1] = "OR HE WILL KEEP";
    lines[2] = "HURTING HUMANS!";
    dialogs_obj.show_timed_dialog("betabot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "NO BETA, THIS";
    lines[1] = "ISN'T HOW WE DO.";
    lines[2] = "WE'RE NOT KILLERS.";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "VERY WELL, BUT";
    lines[1] = "DON'T COUNT ME";
    lines[2] = "TO COME HOME.";
    dialogs_obj.show_timed_dialog("betabot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "THIS IS NOT THE END.";
    lines[1] = "WE'LL MEET AGAIN";
    lines[2] = "ROCK.. AND BETA.";
    dialogs_obj.show_timed_dialog("destrin.png", true, lines, TIMED_DIALOG_DURATION, false);
}

void ending::boss_betabot(graphicsLib_gSurface &last_stage_bg, graphicsLib_gSurface &destrin_sprites, st_position &destrin_pos, st_position &p_pos)
{
    std::string lines[3];
    dialogs dialogs_obj;
    st_rectangle destrin_frame(86, 0, 30, destrin_sprites.height);

    lines[0] = "THIS IS IMPOSSIBLE!!";
    lines[1] = "YOU ARE TOO STRONG,";
    lines[2] = "YOU ARE A MONSTER!";
    dialogs_obj.show_timed_dialog("destrin.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "I AM, AS STRONG AS";
    lines[1] = "NEEDED TO PROTECT.";
    lines[2] = "MAKE YOUR PRAYERS.";
    dialogs_obj.show_timed_dialog("betabot.png", true, lines, TIMED_DIALOG_DURATION, false);

    gameControl.set_player_anim_type(ANIM_TYPE_ATTACK, 0);
    graphLib.showSurface(&last_stage_bg);
    gameControl.show_player(0);
    graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
    draw_lib.update_screen();
    input.waitTime(20);

    lines[0] = "NO, ROBOTS CAN'T";
    lines[1] = "HARM US HUMANS!";
    lines[2] = "STOP NOW!!";
    dialogs_obj.show_timed_dialog("destrin.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "I'M MORE THAN A";
    lines[1] = "MERE 'ROBOT'.";
    lines[2] = "NOW DIE!";
    dialogs_obj.show_timed_dialog("betabot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "WAIT BETA!";
    lines[1] = "DON'T DO IT.";
    lines[2] = "";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);

    // rockbot teleports in
    gameControl.get_player(1)->reset_jump();
    gameControl.get_player(1)->set_map(gameControl.get_player(0)->get_map());
    gameControl.get_player(1)->set_position(st_position(gameControl.get_player(0)->getPosition().x+100, -10));
    gameControl.get_player(1)->set_animation_type(ANIM_TYPE_TELEPORT);
    gameControl.get_player(1)->set_teleport_minimal_y(gameControl.get_player(0)->getPosition().y-TILESIZE);

    //std::cout << ">> p1->hit_ground: " << gameControl.get_player(1)->hit_ground() << std::endl;
    while (true) {
        //std::cout << ">> wait for p1 to hit ground <<" << std::endl;
        gameControl.get_player(1)->gravity(false);
        graphLib.showSurface(&last_stage_bg);
        graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
        gameControl.show_player(0);
        gameControl.show_player(1);
        draw_lib.update_screen();
        input.waitTime(5);
        if (gameControl.get_player(1)->getPosition().y >= gameControl.get_player(0)->getPosition().y-TILESIZE && gameControl.get_player(1)->hit_ground() == true) {
            soundManager.play_sfx(SFX_PLAYER_JUMP);
            break;
        }
    }

    if (gameControl.get_player(1)->get_real_position().x > RES_W/3+TILESIZE*3) {
        gameControl.get_player(1)->set_animation_type(ANIM_TYPE_WALK);
    }
    while (gameControl.get_player(1)->get_real_position().x > RES_W/3+TILESIZE*3) {
        gameControl.set_player_position(st_position(gameControl.get_player(1)->getPosition().x-3, gameControl.get_player(1)->getPosition().y), 1);
        graphLib.showSurface(&last_stage_bg);
        graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
        gameControl.show_player(0);
        gameControl.show_player(1);
        draw_lib.update_screen();
        input.waitTime(20);
    }

    gameControl.get_player(1)->set_animation_type(ANIM_TYPE_SHIELD);
    graphLib.showSurface(&last_stage_bg);
    graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
    gameControl.show_player(0);
    gameControl.show_player(1);
    draw_lib.update_screen();
    input.waitTime(1000);

    lines[0] = "ROCK... ARE YOU,";
    lines[1] = "PROTECTING THIS..";
    lines[2] = "THIS VILAIN!?";
    dialogs_obj.show_timed_dialog("betabot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "IM AM ACTUALLY";
    lines[1] = "PROTECTING YOU BROTHER.";
    lines[2] = "";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);


    lines[0] = "IF YOU KILL A HUMAN, EVEN";
    lines[1] = "A BAD ONE, PEOPLE WILL";
    lines[2] = "HATE ALL ROBOTS.";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);


    gameControl.set_player_anim_type(ANIM_TYPE_STAND, 0);
    gameControl.set_player_anim_type(ANIM_TYPE_SHIELD, 1);
    graphLib.showSurface(&last_stage_bg);
    graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
    gameControl.show_player(0);
    gameControl.show_player(1);
    draw_lib.update_screen();

    lines[0] = "UNDERSTOOD. BUT REMEMBER";
    lines[1] = "THIS: ANYONE HE HURTS,";
    lines[2] = "IS YOUR FAULT.";
    dialogs_obj.show_timed_dialog("betabot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "I KNOW AND WON'T";
    lines[1] = "ALLOW IT.";
    lines[2] = "WHERE WILL YOU GO?";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "FOLLOW THE WIND,";
    lines[1] = "TO BE FORGOTTEN,";
    lines[2] = "TO BE FREE.";
    dialogs_obj.show_timed_dialog("betabot.png", true, lines, TIMED_DIALOG_DURATION, false);

    gameControl.set_player_anim_type(ANIM_TYPE_TELEPORT, 0);
    gameControl.set_player_anim_type(ANIM_TYPE_STAND, 1);

    // BETABOT TELEPORTS OUT
    while (p_pos.y > -TILESIZE*3) {
        p_pos.y -= 8;
        gameControl.set_player_position(p_pos, 0);
        graphLib.showSurface(&last_stage_bg);
        graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
        gameControl.show_player(0);
        gameControl.show_player(1);
        draw_lib.update_screen();
        input.waitTime(20);
    }
    gameControl.set_player_anim_type(ANIM_TYPE_STAND, 0);
    p_pos.y = 100;
    gameControl.set_player_position(p_pos, 0);
}

void ending::boss_candybot(graphicsLib_gSurface &last_stage_bg, graphicsLib_gSurface &destrin_sprites, st_position &destrin_pos, st_position &p_pos)
{
    std::string lines[3];
    dialogs dialogs_obj;
    st_rectangle destrin_frame(86, 0, 30, destrin_sprites.height);

    gameControl.set_player_position(st_position(RES_W/3+TILESIZE*3, -TILESIZE*3), 1);

    lines[0] = "HOW COULD SUCK A";
    lines[1] = "WEAK GIRL LIKE YOU";
    lines[2] = "DEFEAT ME?";
    dialogs_obj.show_timed_dialog("destrin.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "MISTER, YOU MUST KNOW";
    lines[1] = "THAT LOVE WILL ALWAYS";
    lines[2] = "PREVAIL OVER EVILDOERS.";
    dialogs_obj.show_timed_dialog("player3.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "WHAT WILL YOU DO NOW?";
    lines[1] = "PUT ME INTO A PRISION?";
    lines[2] = "I WILL ESCAPE SOON";
    dialogs_obj.show_timed_dialog("destrin.png", true, lines, TIMED_DIALOG_DURATION, false);


    // ROCKBOT teleports in
    gameControl.get_player(1)->reset_jump();
    gameControl.get_player(1)->set_map(gameControl.get_player(0)->get_map());
    gameControl.get_player(1)->set_position(st_position(gameControl.get_player(0)->getPosition().x+40, -10));
    gameControl.get_player(1)->set_animation_type(ANIM_TYPE_TELEPORT);
    gameControl.get_player(1)->set_direction(ANIM_DIRECTION_RIGHT);
    gameControl.get_player(1)->set_teleport_minimal_y(gameControl.get_player(0)->getPosition().y-TILESIZE);

    //std::cout << ">> p1->hit_ground: " << gameControl.get_player(1)->hit_ground() << std::endl;
    while (true) {
        //std::cout << ">> wait for p1 to hit ground <<" << std::endl;
        gameControl.get_player(1)->gravity(false);
        graphLib.showSurface(&last_stage_bg);
        graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
        gameControl.show_player(0);
        gameControl.show_player(1);
        draw_lib.update_screen();
        input.waitTime(5);
        if (gameControl.get_player(1)->getPosition().y >= gameControl.get_player(0)->getPosition().y-TILESIZE && gameControl.get_player(1)->hit_ground() == true) {
            soundManager.play_sfx(SFX_PLAYER_JUMP);
            break;
        }
    }

    gameControl.set_player_anim_type(ANIM_TYPE_STAND, 1);
    graphLib.showSurface(&last_stage_bg);
    gameControl.show_player(1);
    gameControl.show_player(0);
    graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
    draw_lib.update_screen();
    input.waitTime(20);

    lines[0] = "YOU CAN ESPACE,";
    lines[1] = "BUT ME AND MY FRIENDS";
    lines[2] = "WILL DEFEAT YOU AGAIN.";
    dialogs_obj.show_timed_dialog("player3.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "IT IS OVER, KANOTUS.";
    lines[1] = "YOU CAN'T WIN OVER";
    lines[2] = "US ALL.";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "THIS IS NOT THE END.";
    lines[1] = "I WILL FIND A WAY TO";
    lines[2] = "FINISH YOU ALL!!";
    dialogs_obj.show_timed_dialog("destrin.png", true, lines, TIMED_DIALOG_DURATION, false);
}

void ending::boss_kittybot(graphicsLib_gSurface &last_stage_bg, graphicsLib_gSurface &destrin_sprites, st_position &destrin_pos, st_position &p_pos)
{
    std::string lines[3];
    dialogs dialogs_obj;
    st_rectangle destrin_frame(86, 0, 30, destrin_sprites.height);

    lines[0] = "I CAN'T BELIEVE";
    lines[1] = "I WAS DEFEATED BY";
    lines[2] = "A MERE THUG!";
    dialogs_obj.show_timed_dialog("destrin.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "THUG YOU CALL ME,";
    lines[1] = "BUT THE TRUE THIEF";
    lines[2] = "LIES BEFORE ME.";
    dialogs_obj.show_timed_dialog("player4.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "WELL THEN,";
    lines[1] = "LETS PUT AN END TO";
    lines[2] = "OUR DANCE.";
    dialogs_obj.show_timed_dialog("player4.png", true, lines, TIMED_DIALOG_DURATION, false);


    gameControl.set_player_anim_type(ANIM_TYPE_ATTACK, 0);
    graphLib.showSurface(&last_stage_bg);
    gameControl.show_player(0);
    graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
    draw_lib.update_screen();
    input.waitTime(20);

    lines[0] = "WHAT ARE YOU DOING?";
    lines[1] = "YOU CAN'T ATTACK A";
    lines[2] = "HUMAN. STOP NOW!!";
    dialogs_obj.show_timed_dialog("destrin.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "I WAS PROGRAMMED BY";
    lines[1] = "A PERSON THAT DID";
    lines[2] = "NOT CARED ABOUT LAWS.";
    dialogs_obj.show_timed_dialog("player4.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "STOP!";
    lines[1] = "I WON'T ALLOW YOU TO";
    lines[2] = "HARM HIM.";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);

    // rockbot teleports in
    gameControl.get_player(1)->reset_jump();
    gameControl.get_player(1)->set_map(gameControl.get_player(0)->get_map());
    gameControl.get_player(1)->set_position(st_position(gameControl.get_player(0)->getPosition().x+100, -10));
    gameControl.get_player(1)->set_animation_type(ANIM_TYPE_TELEPORT);
    gameControl.get_player(1)->set_teleport_minimal_y(gameControl.get_player(0)->getPosition().y-TILESIZE);

    //std::cout << ">> p1->hit_ground: " << gameControl.get_player(1)->hit_ground() << std::endl;
    while (true) {
        //std::cout << ">> wait for p1 to hit ground <<" << std::endl;
        gameControl.get_player(1)->gravity(false);
        graphLib.showSurface(&last_stage_bg);
        graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
        gameControl.show_player(0);
        gameControl.show_player(1);
        draw_lib.update_screen();
        input.waitTime(5);
        if (gameControl.get_player(1)->getPosition().y >= gameControl.get_player(0)->getPosition().y-TILESIZE && gameControl.get_player(1)->hit_ground() == true) {
            soundManager.play_sfx(SFX_PLAYER_JUMP);
            break;
        }
    }

    if (gameControl.get_player(1)->get_real_position().x > RES_W/3+TILESIZE*3) {
        gameControl.get_player(1)->set_animation_type(ANIM_TYPE_WALK);
    }
    while (gameControl.get_player(1)->get_real_position().x > RES_W/3+TILESIZE*3) {
        gameControl.set_player_position(st_position(gameControl.get_player(1)->getPosition().x-3, gameControl.get_player(1)->getPosition().y), 1);
        graphLib.showSurface(&last_stage_bg);
        graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
        gameControl.show_player(0);
        gameControl.show_player(1);
        draw_lib.update_screen();
        input.waitTime(20);
    }

    gameControl.get_player(1)->set_animation_type(ANIM_TYPE_SHIELD);
    graphLib.showSurface(&last_stage_bg);
    graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
    gameControl.show_player(0);
    gameControl.show_player(1);
    draw_lib.update_screen();
    input.waitTime(1000);

    lines[0] = "WHAT ARE YOU DOING,";
    lines[1] = "YOUR IMBECILE?";
    lines[2] = "LET ME JUST DO IT.";
    dialogs_obj.show_timed_dialog("player4.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "I CAN'T.";
    lines[1] = "THE LAWS OF ROBOTICS";
    lines[2] = "MAKE ME PROTECT HUMANS.";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);


    lines[0] = "I HAVE NO CHOISE";
    lines[1] = "AND NEITHER SHOULD";
    lines[2] = "YOU, GIRL.";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);


    gameControl.set_player_anim_type(ANIM_TYPE_STAND, 0);
    gameControl.set_player_anim_type(ANIM_TYPE_SHIELD, 1);
    graphLib.showSurface(&last_stage_bg);
    graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
    gameControl.show_player(0);
    gameControl.show_player(1);
    draw_lib.update_screen();

    lines[0] = "VERY VELL, I WILL";
    lines[1] = "LET YOU PLAY THIS GAME.";
    lines[2] = "GET HIM AND LEAVE, NOW!";
    dialogs_obj.show_timed_dialog("player4.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "WHY DO YOU NOT HAVE";
    lines[1] = "THE ROBOTICS LAWS IN";
    lines[2] = "YOUR CODE?";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "THAT IS FOR ME TO KNOW,";
    lines[1] = "AND FOR THIS TRASH THERE";
    lines[2] = "TO FEAR.";
    dialogs_obj.show_timed_dialog("player4.png", true, lines, TIMED_DIALOG_DURATION, false);

    gameControl.set_player_anim_type(ANIM_TYPE_TELEPORT, 0);
    gameControl.set_player_anim_type(ANIM_TYPE_STAND, 1);

    // KITTYBOT TELEPORTS OUT
    while (p_pos.y > -TILESIZE*3) {
        p_pos.y -= 8;
        gameControl.set_player_position(p_pos, 0);
        graphLib.showSurface(&last_stage_bg);
        graphLib.showSurfaceRegionAt(&destrin_sprites, destrin_frame, destrin_pos);
        gameControl.show_player(0);
        gameControl.show_player(1);
        draw_lib.update_screen();
        input.waitTime(20);
    }
    gameControl.set_player_anim_type(ANIM_TYPE_STAND, 0);
    p_pos.y = 100;
    gameControl.set_player_position(p_pos, 0);
}

void ending::train()
{
	unsigned int end_timer;
	int pos_layer_bg=0;
	int pos_layer_fence=0;
	unsigned int check_timer;
	int train_move=0;
	int played_sfx = 0;
	int posx_adjust = 0;
	int pos_bushes_1 = 0;
	int pos_bushes_2 = 0;


    graphLib.set_colormap(-1);




	soundManager.stop_music();
	soundManager.load_music("train.mod");
	soundManager.play_music();


	pos_bushes_1 = RES_W + _bushes_surface.width;
	pos_bushes_2 = RES_W*1.5 + _bushes_surface.width;


	graphLib.copyArea(st_rectangle(0, 0, RES_W, _bg_surface.height), st_position(0, 0), &_bg_surface, &graphLib.gameScreen);
	graphLib.copyArea(st_rectangle(0, 0, RES_W, _fence_surface.height), st_position(0, _bg_surface.height), &_fence_surface, &graphLib.gameScreen);
	graphLib.copyArea(st_rectangle(0, 0, RES_W, _train_surface.height), st_position(0, _bg_surface.height+_fence_surface.height), &_train_surface, &graphLib.gameScreen);
    draw_lib.update_screen();


	end_timer = timer.getTimer()+10000;
	check_timer = timer.getTimer()+2000;
	graphLib.blank_screen();

	gameControl.set_player_anim_type(ANIM_TYPE_STAND, 0);
	gameControl.set_player_direction(ANIM_DIRECTION_RIGHT, 0);

    st_position p_pos = st_position(RES_W/6, _bg_surface.height+_fence_surface.height - gameControl.get_player_size(0).height);
    gameControl.set_player_position(p_pos, 0);

    //std::cout << "p_pos.x: " << p_pos.x << ", p_pos.y: " << p_pos.y << std::endl;


	while (timer.getTimer() < end_timer) {
		// move background
		pos_layer_bg++;
		if (pos_layer_bg >= _bg_surface.width-RES_W) {
			pos_layer_bg = 0;
		}
		// move fence
		pos_layer_fence += 4;
		if (pos_layer_fence >= _fence_surface.width-RES_W) {
			pos_layer_fence = 0;
		}

		pos_bushes_1 -= 12;
		if (pos_bushes_1 < -_bushes_surface.width) {
			pos_bushes_1 = RES_W + _bushes_surface.width;
		}

		pos_bushes_2 -= 12;
		if (pos_bushes_2 < -_bushes_surface.width) {
			pos_bushes_2 = RES_W + _bushes_surface.width;
		}


		graphLib.copyArea(st_rectangle(pos_layer_bg, 0, RES_W, _bg_surface.height), st_position(0, 0), &_bg_surface, &graphLib.gameScreen);
		graphLib.copyArea(st_rectangle(0, 0, RES_W, _bg_surface.height), st_position(0, 0), &_bg_surface, &graphLib.gameScreen);
		graphLib.copyArea(st_rectangle(pos_layer_fence, 0, RES_W, _fence_surface.height), st_position(0, _bg_surface.height), &_fence_surface, &graphLib.gameScreen);

		if (timer.getTimer() > check_timer) {
			if (train_move == 0) {
				train_move = 1;
				check_timer = timer.getTimer()+100;
			} else {
				train_move = 0;
				check_timer = timer.getTimer()+2000;
			}
		}
		if (train_move == 0) {
			posx_adjust = 0;
			played_sfx = 0;
		} else {
			if (!played_sfx) {
				soundManager.play_sfx(SFX_TRAIN_CHUNK);
			}
			played_sfx = 1;
			posx_adjust = 2;
		}

		graphLib.copyArea(st_rectangle(posx_adjust, 0, RES_W, _train_surface.height), st_position(0, _bg_surface.height+_fence_surface.height), &_train_surface, &graphLib.gameScreen);
		graphLib.copyArea(st_rectangle(pos_layer_fence, 0, RES_W, _fence_surface.height), st_position(0, _bg_surface.height), &_fence_surface, &graphLib.gameScreen);


		gameControl.set_player_position(st_position(p_pos.x - posx_adjust, p_pos.y), 0);
        //std::cout << "p_pos.x: " << p_pos.x << ", p_pos.y: " << p_pos.y << std::endl;
		gameControl.show_player(0);

		graphLib.copyArea(st_rectangle(0, 0, _bushes_surface.width, _bushes_surface.height), st_position(pos_bushes_1, RES_H-_bushes_surface.height), &_bushes_surface, &graphLib.gameScreen);
		graphLib.copyArea(st_rectangle(0, 0, _bushes_surface.width, _bushes_surface.height), st_position(pos_bushes_2, RES_H-_bushes_surface.height), &_bushes_surface, &graphLib.gameScreen);

        draw_lib.update_screen();

		input.waitTime(30);
	}

	short int walk_total = p_pos.x + RES_W/2;

	// walk to the right-side of screen
	gameControl.set_player_anim_type(ANIM_TYPE_WALK, 0);
	while (p_pos.x < walk_total) {
		p_pos.x += 3;
		gameControl.set_player_position(st_position(p_pos), 0);
		graphLib.copyArea(st_rectangle(0, 0, RES_W, _bg_surface.height), st_position(0, 0), &_bg_surface, &graphLib.gameScreen);
		graphLib.copyArea(st_rectangle(0, 0, RES_W, _fence_surface.height), st_position(0, _bg_surface.height), &_fence_surface, &graphLib.gameScreen);
		graphLib.copyArea(st_rectangle(0, 0, RES_W, _train_surface.height), st_position(0, _bg_surface.height+_fence_surface.height), &_train_surface, &graphLib.gameScreen);
		gameControl.show_player(0);
		graphLib.copyArea(st_rectangle(0, 0, _bushes_surface.width, _bushes_surface.height), st_position(pos_bushes_1, RES_H-_bushes_surface.height), &_bushes_surface, &graphLib.gameScreen);
		graphLib.copyArea(st_rectangle(0, 0, _bushes_surface.width, _bushes_surface.height), st_position(pos_bushes_2, RES_H-_bushes_surface.height), &_bushes_surface, &graphLib.gameScreen);
        draw_lib.update_screen();
		input.waitTime(20);
	}

	// jump up
	float jump_speed = 0.0;
	gameControl.set_player_anim_type(ANIM_TYPE_JUMP, 0);
	int jump_up = 1;
	float acceleration = 0.12;
	short max_speed = 9;
	jump_speed = max_speed;

	while (p_pos.y < RES_H) { // @TODO: get_player_size, use height
		if (jump_up == 1) {
			jump_speed -= jump_speed*acceleration;
			p_pos.y -= jump_speed;
		} else {
			if (jump_speed <= 0) {
				jump_speed = 1;
			}
			jump_speed += jump_speed*acceleration;
			p_pos.y += jump_speed;
		}
		gameControl.set_player_position(st_position(p_pos), 0);

		graphLib.copyArea(st_rectangle(0, 0, RES_W, _bg_surface.height), st_position(0, 0), &_bg_surface, &graphLib.gameScreen);
		graphLib.copyArea(st_rectangle(0, 0, RES_W, _fence_surface.height), st_position(0, _bg_surface.height), &_fence_surface, &graphLib.gameScreen);
		graphLib.copyArea(st_rectangle(0, 0, RES_W, _train_surface.height), st_position(0, _bg_surface.height+_fence_surface.height), &_train_surface, &graphLib.gameScreen);
		gameControl.show_player(0);
		graphLib.copyArea(st_rectangle(0, 0, _bushes_surface.width, _bushes_surface.height), st_position(pos_bushes_1, RES_H-_bushes_surface.height), &_bushes_surface, &graphLib.gameScreen);
		graphLib.copyArea(st_rectangle(0, 0, _bushes_surface.width, _bushes_surface.height), st_position(pos_bushes_2, RES_H-_bushes_surface.height), &_bushes_surface, &graphLib.gameScreen);
        draw_lib.update_screen();

		if (jump_up == 1 && jump_speed <= 1.0) {
			jump_up = 0;
		}

		input.waitTime(20);
	}
}

void ending::lab() const
{
	graphicsLib_gSurface lab_bg, kanotus_surface, candybot_surface;
	st_position p_pos;
    int y_adjust = 60;


	graphLib.surfaceFromFile(std::string(FILEPATH+"data/images/scenes/lab_ending.png"), &lab_bg);
	graphLib.surfaceFromFile(std::string(FILEPATH+"data/images/sprites/canotus.png"), &kanotus_surface);
	graphLib.surfaceFromFile(std::string(FILEPATH+"data/images/sprites/candybot_ending.png"), &candybot_surface);


    graphLib.set_colormap(-3);
    graphLib.blank_screen();
    graphLib.set_colormap_current(&lab_bg);
    graphLib.copyArea(st_rectangle(0, 0, lab_bg.width, lab_bg.height), st_position(0, y_adjust+10-graphLib.RES_DIFF_H), &lab_bg, &graphLib.gameScreen);
    graphLib.copyArea(st_rectangle(0, 0, 21, 24), st_position(58, y_adjust+82), &kanotus_surface, &graphLib.gameScreen);
    graphLib.copyArea(st_rectangle(0, 0, 16, 26), st_position(174, y_adjust+80), &candybot_surface, &graphLib.gameScreen);

    gameControl.set_player_position(st_position(RES_W+TILESIZE*2, y_adjust+77), 0);
	gameControl.set_player_direction(ANIM_DIRECTION_LEFT, 0);
	gameControl.set_player_anim_type(ANIM_TYPE_WALK, 0);
	p_pos = gameControl.get_player_position(0);

	while (p_pos.x > RES_W/2-50) {
		p_pos.x -= 3;
        graphLib.set_colormap_current(&lab_bg);
        graphLib.copyArea(st_rectangle(0, 0, lab_bg.width, lab_bg.height), st_position(0, y_adjust+10-graphLib.RES_DIFF_H), &lab_bg, &graphLib.gameScreen);
        graphLib.copyArea(st_rectangle(0, 0, 21, 24), st_position(58, y_adjust+82), &kanotus_surface, &graphLib.gameScreen);
		if (p_pos.x > 172) {
            graphLib.copyArea(st_rectangle(0, 0, 16, 26), st_position(174, y_adjust+80), &candybot_surface, &graphLib.gameScreen);
		} else {
            graphLib.copyArea(st_rectangle(16, 0, 16, 26), st_position(174, y_adjust+80), &candybot_surface, &graphLib.gameScreen);
		}
		gameControl.set_player_position(p_pos, 0);
		gameControl.show_player(0);
        draw_lib.update_screen();
		input.waitTime(20);
	}
	gameControl.set_player_anim_type(ANIM_TYPE_STAND, 0);
    graphLib.set_colormap_current(&lab_bg);
    graphLib.copyArea(st_rectangle(0, 0, lab_bg.width, lab_bg.height), st_position(0, y_adjust+10-graphLib.RES_DIFF_H), &lab_bg, &graphLib.gameScreen);
    graphLib.copyArea(st_rectangle(64, 0, 21, 24), st_position(58, y_adjust+82), &kanotus_surface, &graphLib.gameScreen);
    graphLib.copyArea(st_rectangle(16, 0, 16, 26), st_position(174, y_adjust+80), &candybot_surface, &graphLib.gameScreen);
	gameControl.show_player(0);
    draw_lib.update_screen();
	input.waitTime(1000);

	dialogs dialogs_obj;
	std::string lines[3];
    lines[0] = "ROCK, YOU ARE BACK!";
    lines[1] = "YOU SAVED THE CITY AND";
    lines[2] = "THEY CALL YOU A HERO.";
    dialogs_obj.show_timed_dialog("candybot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "THAT IS WHAT I WANTED.";
    lines[1] = "BUT STILL, I CAN'T";
    lines[2] = "HELP AND FELL SAD.";
    dialogs_obj.show_timed_dialog("rockbot_sad.png", true, lines, TIMED_DIALOG_DURATION, false);

	lines[0] = "WHY?";
	lines[1] = "AND WHERE IS BETA?";
	lines[2] = "";
    dialogs_obj.show_timed_dialog("candybot.png", true, lines, TIMED_DIALOG_DURATION, false);

	lines[0] = "HE...HE...";
    lines[1] = "HE DECIDED TO FOLLOW";
    lines[2] = "HIS OWN PATH.";
    dialogs_obj.show_timed_dialog("rockbot_sad.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "I MADE YOU WITH FREE";
    lines[1] = "WILL AND THINKING,";
    lines[2] = "SO LETS RESPECT HIM.";
    dialogs_obj.show_timed_dialog("canotus_face.png", true, lines, TIMED_DIALOG_DURATION, false);

    graphLib.set_colormap(-1);
}

void ending::look_sky_betabot() const
{
    // twilight background
    graphicsLib_gSurface background;
    graphicsLib_gSurface betabot_looking_up;
    graphicsLib_gSurface kittybot;

    graphLib.surfaceFromFile(std::string(FILEPATH+"data/images/backgrounds/twilight.png"), &background);
    graphLib.surfaceFromFile(std::string(FILEPATH+"data/images/sprites/p2_looking_up.png"), &betabot_looking_up);
    graphLib.surfaceFromFile(std::string(FILEPATH+"data/images/sprites/kittybot.png"), &kittybot);
    graphLib.showSurface(&background);
    graphLib.showSurfaceAt(&betabot_looking_up, st_position(81, 192-betabot_looking_up.height), false); // 81, 192
    graphLib.showSurfaceRegionAt(&kittybot, st_rectangle(0, 0, 19, 28), st_position(290, 192-kittybot.height)); // 290, 192
    graphLib.updateScreen();
    timer.delay(2000);

    dialogs dialogs_obj;
    std::string lines[3];
    // 19 x 28

    lines[0] = "MEOWN. HEY MISTER!";
    lines[1] = "WHY ARE YOU HERE,";
    lines[2] = "LOOKING AT THE SKY?";
    dialogs_obj.show_timed_dialog("kittybot.png", true, lines, TIMED_DIALOG_DURATION, false);

    gameControl.set_player_position(st_position(81, 192-29), 0);
    gameControl.set_player_anim_type(ANIM_TYPE_STAND, 0);
    gameControl.set_player_direction(ANIM_DIRECTION_RIGHT, 0);
    graphLib.showSurface(&background);
    gameControl.show_player(0);
    graphLib.showSurfaceRegionAt(&kittybot, st_rectangle(19, 0, 38, 28), st_position(290, 192-kittybot.height)); // 290, 192
    draw_lib.update_screen();
    input.waitTime(1000);

    lines[0] = "I AM NOW FREE,";
    lines[1] = "BUT WHAT SHOULD I DO";
    lines[2] = "WITH SUCH THING?";
    dialogs_obj.show_timed_dialog("betabot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "OH, WORRY NOT!";
    lines[1] = "AS, YOU SEE, I AM";
    lines[2] = "PLENTY OF IDEAS.";
    dialogs_obj.show_timed_dialog("kittybot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "WHO ARE YOU,";
    lines[1] = "STRAY ROBOT?";
    lines[2] = "";
    dialogs_obj.show_timed_dialog("betabot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "KITTYBOT IS THE NAME,";
    lines[1] = "AND YOU MISTER,";
    lines[2] = "ARE WHOM?";
    dialogs_obj.show_timed_dialog("kittybot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "BETA....";
    lines[1] = "NO, FROM NOW ON";
    lines[2] = "THE NAME IS...";
    dialogs_obj.show_timed_dialog("betabot.png", true, lines, TIMED_DIALOG_DURATION, false);

    timer.delay(2000);
}

void ending::look_sky_rockbot() const
{
    // twilight background
    graphicsLib_gSurface background;
    graphicsLib_gSurface betabot_looking_up;

    graphLib.surfaceFromFile(std::string(FILEPATH+"data/images/backgrounds/night.png"), &background);
    graphLib.surfaceFromFile(std::string(FILEPATH+"data/images/sprites/p1_looking_up.png"), &betabot_looking_up);
    graphLib.showSurface(&background);
    graphLib.showSurfaceAt(&betabot_looking_up, st_position(222, 192-betabot_looking_up.height), false); // 81, 192
    graphLib.updateScreen();
    timer.delay(2000);

    dialogs dialogs_obj;
    std::string lines[3];

    lines[0] = "BROTHER...";
    lines[1] = "WHY DID YOU CHOSE";
    lines[2] = "THIS VENGEANCE PATH?";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "I CAN'T UNDERSTAND.";
    lines[1] = "WE WERE SUPPOSED TO";
    lines[2] = "BE HEROES, BOTH OF US.";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "WILL I HAVE TO FIGHT";
    lines[1] = "YOU IN THE FUTURE,";
    lines[2] = "TO PROTECT HUMANS?";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);

    lines[0] = "WILL YOU FIGHT";
    lines[1] = "ME FOR WHAT YOU";
    lines[2] = "BELIEVE?";
    dialogs_obj.show_timed_dialog("rockbot.png", true, lines, TIMED_DIALOG_DURATION, false);

    timer.delay(2000);
}

void ending::bosses() const
{
    /// @TODO - create function that reads only the bosses ids for each stage
    graphLib.blank_screen();
    graphLib.updateScreen();

    soundManager.stop_music();
	soundManager.load_music("ending_bosses.mod");
	soundManager.play_music();
    graphLib.start_stars_animation();
	/// @TODO - copy from boss_intro
    boss_credits(APEBOT, fio.read_stage_boss_id(APEBOT, stage_data));
    boss_credits(TECHNOBOT, fio.read_stage_boss_id(TECHNOBOT, stage_data));
    boss_credits(DYNAMITEBOT, fio.read_stage_boss_id(DYNAMITEBOT, stage_data));
    boss_credits(DAISIEBOT, fio.read_stage_boss_id(DAISIEBOT, stage_data));
    boss_credits(SPIKEBOT, fio.read_stage_boss_id(SPIKEBOT, stage_data));
    boss_credits(MUMMYBOT, fio.read_stage_boss_id(MUMMYBOT, stage_data));
    boss_credits(SEAHORSEBOT, fio.read_stage_boss_id(SEAHORSEBOT, stage_data));
    boss_credits(MAGEBOT, fio.read_stage_boss_id(MAGEBOT, stage_data));
    graphLib.stop_stars_animation();

}

void ending::credits() const
{


	soundManager.stop_music();
	soundManager.load_music("ending.mod");
	soundManager.play_music();

    graphLib.start_stars_animation();


    draw_lib.update_screen();

    draw_lib.show_credits();

    input.wait_keypress();

    graphLib.stop_stars_animation();
#ifdef PC
    std::cout << "LEAVE #4" << std::endl;
    std::fflush(stdout);
    gameControl.leave_game();
#endif
}



void ending::boss_credits(int pos_n, short npc_id) const
{
    int text_x = RES_W*0.5 - 85;
    std::string botname;
    std::string concept_author;
    std::string design_author;



    if (pos_n == SPIKEBOT) {
        botname = "NO. 005 - SPIKE BOT";
        concept_author = "CONCEPT: IURI FIEDORUK";
        design_author = "DESIGN: IURI FIEDORUK";
    } else if (pos_n == DAISIEBOT) {
        botname = "NO. 004 - DAISIE BOT";
        concept_author = "CONCEPT: ARIS KSF";
        design_author = "DESIGN: ARIS KSF";
    } else if (pos_n == DYNAMITEBOT) {
        botname = "NO. 003 - DYNAMITE BOT";
        concept_author = "CONCEPT: KOBOLD SACCOL";
        design_author = "DESIGN: IURI FIEDORUK";
    } else if (pos_n == TECHNOBOT) {
        botname = "NO. 002 - TECHNO BOT";
        concept_author = "CONCEPT: CHIKAGO SACCOL";
        design_author = "DESIGN: IURI FIEDORUK";
    } else if (pos_n == MUMMYBOT) {
        botname = "NO. 006 - MUMMY BOT";
        concept_author = "CONCEPT: ARIS KSF";
        design_author = "DESIGN: ARIS KSF";
    } else if (pos_n == APEBOT) {
        botname = "NO. 001 - APE BOT";
        concept_author = "CONCEPT: IVAN FIEDORUK";
        design_author = "DESIGN: IURI FIEDORUK";
    } else if (pos_n == SEAHORSEBOT) {
        botname = "NO. 007 - SEAHORSE BOT";
        concept_author = "CONCEPT: ARIS KSF";
        design_author = "DESIGN: IURI FIEDORUK";
    } else if (pos_n == MAGEBOT) {
        botname = "NO. 008 - MAGE BOT";
        concept_author = "CONCEPT: ARIS KSF";
        design_author = "DESIGN: ARIS K.S.F.";
    } else {
        return;
    }

    draw_lib.show_boss_intro_sprites(npc_id, false);

    graphLib.draw_progressive_text(text_x, RES_H/2-15, botname, false);
    graphLib.draw_progressive_text(text_x, RES_H/2-3, concept_author, false);
    graphLib.draw_progressive_text(text_x, RES_H/2+9, design_author, false);

    graphLib.wait_and_update_screen(2500);
}

void ending::set_boss_pos(st_position pos)
{
    _boss_pos = pos;
}
