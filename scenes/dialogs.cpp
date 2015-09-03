#include <cstring>
#include "dialogs.h"


extern std::string FILEPATH;

#include "graphicslib.h"
extern graphicsLib graphLib;

#include "inputlib.h"
extern inputLib input;

#include "graphic/draw.h"
extern draw draw_lib;

#include "timerlib.h"
extern timerLib timer;


#include "graphic/option_picker.h"


extern CURRENT_FILE_FORMAT::st_game_config game_config;

extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_stage stage_data;
extern CURRENT_FILE_FORMAT::st_save game_save;

dialogs::dialogs() : is_showing_dialog_bg(false)
{
    /// @TODO - temporary configurations should not be placed in st_config
    //if (game_config.two_players == 2 || game_config.selected_player == 1) {

    //std::cout << "DIALOGS::CONSTRUCTOR - " << game_save.selected_player << std::endl;

    if (game_save.selected_player == PLAYER_ROCKBOT) {
		player_name = "Rockbot";
		player_face = "rockbot.png";
    } else if (game_save.selected_player == PLAYER_BETABOT) {
		player_name = "Betabot";
		player_face = "betabot.png";
    } else if (game_save.selected_player == PLAYER_CANDYBOT) {
        player_name = "Candybot";
        player_face = "candybot.png";
    } else if (game_save.selected_player == PLAYER_KITTYBOT) {
        player_name = "Kittybot";
        player_face = "kittybot.png";
    }
}


void dialogs::show_stage_dialog()
{
    //std::cout << "DEBUG - dialogs::show_stage_dialog::stage: " << stage << ", player_n: " << (game_config.selected_player) << std::endl;
    if (strlen(stage_data.intro_dialog.face_graphics_filename) <= 0) {
		return;
	}

    if (strlen(stage_data.intro_dialog.line1[0]) <= 0) {
        return;
    }

	std::string lines[3];
	for (int i=0; i<3; i++) {
        lines[i] = std::string(stage_data.intro_dialog.line1[i]);
	}
    show_dialog(stage_data.intro_dialog.face_graphics_filename, stage_data.intro_dialog.top_side, lines, true);
	for (int i=0; i<3; i++) {
        lines[i] = std::string(stage_data.intro_dialog.answer1[game_save.selected_player][i]);
	}
    show_dialog(game_data.players[game_save.selected_player].face_filename, stage_data.intro_dialog.top_side, lines, true); /// @TODO: create "extern" for player number
    if (strlen(stage_data.intro_dialog.line2[0]) > 0) {
		for (int i=0; i<3; i++) {
            lines[i] = std::string(stage_data.intro_dialog.line2[i]);
		}
        show_dialog(stage_data.intro_dialog.face_graphics_filename, stage_data.intro_dialog.top_side, lines, true);
	}
}



void dialogs::show_boss_dialog()
{
    if (strlen(stage_data.intro_dialog.face_graphics_filename) <= 0) {
		return;
	}
	std::string lines[3];
	for (int i=0; i<3; i++) {
        lines[i] = std::string(stage_data.boss_dialog.line1[i]);
	}
    //std::cout << "stage: " << stage << ", boss_face: '" << stage_data.boss.face_graphics_filename << "'" << std::endl;
    if (strlen(stage_data.boss.face_graphics_filename) <= 0) {
        sprintf(stage_data.boss.face_graphics_filename, "%s", "dr_kanotus.png");
	}
    show_dialog(stage_data.boss.face_graphics_filename, stage_data.boss_dialog.top_side, lines, true);

    for (int i=0; i<3; i++) {
        lines[i] = std::string(stage_data.boss_dialog.answer1[game_save.selected_player][i]);
	}

    if (lines[0].size() > 0) {
        show_dialog(game_data.players[game_save.selected_player].face_filename, stage_data.boss_dialog.top_side, lines, true); /// @TODO: create "extern" for player number
    } else {
        return;
    }

    if (strlen(stage_data.boss_dialog.line2[0]) > 0) {
		for (int i=0; i<3; i++) {
            lines[i] = std::string(stage_data.boss_dialog.line2[i]);
		}
        show_dialog(stage_data.boss.face_graphics_filename, stage_data.boss_dialog.top_side, lines, true);
	}
}

void dialogs::show_dialog(std::string face_file, bool top_side, std::string lines[3], bool show_btn=true)
{
    UNUSED(top_side);
	std::string temp_text;
	char temp_char;

    if (lines[0].size() < 1) {
        return;
    }

    timer.pause();

	draw_dialog_bg(show_btn);
    draw_lib.update_screen();
	st_position dialog_pos = graphLib.get_dialog_pos();
	graphLib.place_face(face_file, st_position(dialog_pos.x+16, dialog_pos.y+16));
    draw_lib.update_screen();

	/// @TODO: usar show_config_bg e hide_config_bg da graphLib - modificar para aceitar centered (que é o atual) ou top ou bottom
	for (int i=0; i<3; i++) {
		for (unsigned int j=0; j<lines[i].size(); j++) {
			temp_char = lines[i].at(j);
			temp_text = "";
			temp_text += temp_char;

			graphLib.draw_text(j*9+(dialog_pos.x+52), i*11+(dialog_pos.y+16), temp_text);
            draw_lib.update_screen();
			input.waitTime(15);
		}
	}

    input.wait_keypress();
    timer.unpause();

}

bool dialogs::show_leave_game_dialog() const
{
    bool res = false;
    bool repeat_menu = true;
    int picked_n = -1;

    timer.pause();

    graphicsLib_gSurface bgCopy;
    graphLib.initSurface(st_size(RES_W, RES_H), &bgCopy);
    graphLib.copyArea(st_position(0, 0), &graphLib.gameScreen, &bgCopy);

    graphLib.show_dialog(0, false);
    st_position dialog_pos = graphLib.get_dialog_pos();
    graphLib.draw_text(dialog_pos.x+30, dialog_pos.y+16, "QUIT GAME?");
    std::vector<std::string> item_list;
    item_list.push_back("YES");
    item_list.push_back("NO");
    option_picker main_picker(false, st_position(dialog_pos.x+40, dialog_pos.y+16+11), item_list, false);
    draw_lib.update_screen();
    while (repeat_menu == true) {
        picked_n = main_picker.pick();
        if (picked_n == 0) {
            res = true;
            repeat_menu = false;
        } else if (picked_n == 1) {
            res = false;
            repeat_menu = false;
        } else {
            main_picker.draw();
        }
    }
    input.clean();
    input.waitTime(200);
    graphLib.copyArea(st_position(0, 0), &bgCopy, &graphLib.gameScreen);
    draw_lib.update_screen();
    timer.unpause();
    return res;
}

void dialogs::show_timed_dialog(std::string face_file, bool is_left, std::string lines[], short delay, bool show_btn=true)
{
    UNUSED(is_left);
	std::string temp_text;
	char temp_char;

    timer.pause();

	draw_dialog_bg(show_btn);
    draw_lib.update_screen();
	st_position dialog_pos = graphLib.get_dialog_pos();
    graphLib.place_face(face_file, st_position(dialog_pos.x+16, dialog_pos.y+16));
    draw_lib.update_screen();

	/// @TODO: usar show_config_bg e hide_config_bg da graphLib - modificar para aceitar centered (que é o atual) ou top ou bottom
	for (int i=0; i<3; i++) {
        for (unsigned int j=0; j<lines[i].size(); j++) {
			temp_char = lines[i].at(j);
			temp_text = "";
			temp_text += temp_char;

            graphLib.draw_text(j*9+(dialog_pos.x+52), i*11+(dialog_pos.y+16), temp_text);
			//graphLib.draw_text(j*9+53, i*11+9, "A");
            draw_lib.update_screen();
			input.waitTime(15);
		}
	}
    input.waitTime(delay);
    timer.unpause();
}





void dialogs::draw_dialog_bg(bool show_btn=true)
{
	if (is_showing_dialog_bg == true) {
		return;
	}
	graphLib.show_dialog(1, show_btn);
}

/*
void armor_edit::fill_armor_abilities() {
    std::string arm_abilities[] = {"Super-Shot", "Laser-Beam", "Always-Charged", "Freeze"};
    std::string legs_abilities[] = {"Double Jump", "Air-Dash", "Wall-Grab"};
    std::string body_abilities[] = {"Half-Damage", "Extended Imunnity", "Spikes Immune", "No Push-Back"};
 */
void dialogs::showGotArmorDialog(e_ARMOR_PIECES armor_type)
{
    std::string type_str = "???";
    std::string ability_str = "???";

    //std::cout << ">> showGotArmorDialog - p4.arms: " << game_data.armor_pieces[game_save.selected_player].special_ability[ARMOR_ARMS] << std::endl;

    int type = game_data.armor_pieces[armor_type].special_ability[game_save.selected_player];
    //std::cout << "player: " << game_save.selected_player << ", armor_type: " << armor_type << ", armor.ability: " << type << std::endl;
    if (armor_type == ARMOR_ARMS) {
        type_str = "THIS IMPROVED ARMS WILL";
        if (type == ARMOR_ABILITY_ARMS_ALWAYSCHARGED) {
            ability_str = "FIRE ALWAYS CHARGED.";
        } else if (type == ARMOR_ABILITY_ARMS_LASERBEAM) {
            ability_str = "CHARGE A LASER BEAM";
        } else if (type == ARMOR_ABILITY_ARMS_SUPERSHOT) {
            ability_str = "FIRE A SUPER-SHOT!";
        } else if (type == ARMOR_ABILITY_ARMS_MISSILE) {
            ability_str = "THROW A HADOUKEN.";
        }
    } else if (armor_type == ARMOR_LEGS) {
        if (type == ARMOR_ABILITY_LEGS_AIRDASH) {
            type_str = "THOSE LIGHTER LEGS";
            ability_str = "DASH IN MIDDLE-AIR.";
        } else if (type == ARMOR_ABILITY_LEGS_DOUBLEJUMP) {
            type_str = "THOSE LIGHTER LEGS";
            ability_str = "EXECUTE DOUBLE JUMP.";
        } else if (type == ARMOR_ABILITY_LEGS_SHORYUKEN) {
            type_str = "HOLD UP AND DASH";
            ability_str = "TO SHURYUKEN!";
        }
    } else {
        type_str = "THIS FORTIFIED BODY WILL";
        if (type == ARMOR_ABILITY_BODY_EXTENDEDIMMUNITY) {
            ability_str = "BE INTANGIBLE MORE TIME.";
        } else if (type == ARMOR_ABILITY_BODY_HALFDAMAGE) {
            ability_str = "TAKE HALF DAMAGE.";
        } else if (type == ARMOR_ABILITY_BODY_NOPUSHBACK) {
            ability_str = "RESIST PUSH-BACK.";
        } else if (type == ARMOR_ABILITY_BODY_SPIKESIMMMUNE) {
            ability_str = "RESIST SPIKES.";
        }
    }



    std::string lines[] = {type_str, "GIVE YOU THE ABILITY TO", ability_str};
    show_dialog("canotus_face.png", true, lines, true);
}
