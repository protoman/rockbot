#include <cstring>
#include "dialogs.h"


extern std::string FILEPATH;

#include "../graphicslib.h"
extern graphicsLib graphLib;

#include "../inputlib.h"
extern inputLib input;

#include "../graphic/draw.h"
extern draw draw_lib;

#include "../timerlib.h"
extern timerLib timer;


#include "../game.h"
extern game gameControl;

#include "../graphic/option_picker.h"
#include "../strings_map.h"

extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_stage stage_data;
extern CURRENT_FILE_FORMAT::st_save game_save;

dialogs::dialogs() : is_showing_dialog_bg(false)
{
    /// @TODO - temporary configurations should not be placed in st_config
    //if (game_config.two_players == 2 || game_config.selected_player == 1) {
}




void dialogs::show_dialog(std::string face_file, bool top_side, std::string lines[FS_DIALOG_LINES], bool show_btn=true)
{
    UNUSED(top_side);
	std::string temp_text;
    std::string temp_char;

    if (lines[0].size() < 1) {
        return;
    }

    gameControl.game_pause();

    draw_dialog_bg();
    draw_lib.update_screen();
	st_position dialog_pos = graphLib.get_dialog_pos();
	graphLib.place_face(face_file, st_position(dialog_pos.x+16, dialog_pos.y+16));
    draw_lib.update_screen();

	/// @TODO: usar show_config_bg e hide_config_bg da graphLib - modificar para aceitar centered (que é o atual) ou top ou bottom
    for (int i=0; i<FS_DIALOG_LINES; i++) {
        std::string line = lines[i];
        graphLib.draw_text(dialog_pos.x+52, i*11+(dialog_pos.y+16), line);


        draw_lib.update_screen();
        timer.delay(50);
    }

    if (show_btn == true) {
        graphLib.show_dialog_button(1);
        draw_lib.update_screen();
    }


    input.clean_confirm_button();
    input.wait_keypress();
    gameControl.game_unpause();

}

bool dialogs::show_leave_game_dialog()
{
    string lines[3];
    lines[0] = strings_map::get_instance()->get_ingame_string(strings_ingame_quitgame);
    return show_yes_no_dialog(lines);
}

bool dialogs::show_yes_no_dialog(string lines[3])
{
    bool res = false;
    bool repeat_menu = true;
    int picked_n = -1;

    gameControl.game_pause();

    input.clean();

    graphicsLib_gSurface bgCopy;
    graphLib.initSurface(st_size(RES_W, RES_H), &bgCopy);
    graphLib.copyArea(st_position(0, 0), &graphLib.gameScreen, &bgCopy);

    graphLib.show_dialog(0);
    st_position dialog_pos = graphLib.get_dialog_pos();

    for (int i=0; i<3; i++) {
        graphLib.draw_text(dialog_pos.x+30, dialog_pos.y+16+15*i, lines[i]);
    }

    std::vector<std::string> item_list;

    item_list.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_yes));
    item_list.push_back(strings_map::get_instance()->get_ingame_string(strings_ingame_no));
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
    timer.delay(200);
    graphLib.copyArea(st_position(0, 0), &bgCopy, &graphLib.gameScreen);
    draw_lib.update_screen();
    if (res == false && gameControl.game_started()) {
        gameControl.game_unpause();
    }

    return res;
}

void dialogs::show_timed_dialog(std::string face_file, bool is_left, std::string lines[], short delay, bool show_btn=true)
{
    UNUSED(is_left);
	std::string temp_text;
	char temp_char;

    gameControl.game_pause();

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
			timer.delay(15);
		}
	}

    if (show_btn == true) {
        graphLib.show_dialog_button(1);
        draw_lib.update_screen();
    }


    timer.delay(delay);
    gameControl.game_unpause();
}





void dialogs::draw_dialog_bg()
{
	if (is_showing_dialog_bg == true) {
		return;
	}
    graphLib.show_dialog(1);
}

void dialogs::showGotArmorDialog(e_ARMOR_PIECES armor_type)
{
    std::string type_str = "???";
    std::string ability_str = "???";

    int type = game_data.armor_pieces[armor_type].special_ability[game_save.selected_player];
    if (armor_type == ARMOR_TYPE_ARMS) {
        type_str = strings_map::get_instance()->get_ingame_string(strings_ingame_gotarmor_type_arms);
        std::string weapon_name(GameMediator::get_instance()->projectile_list.at(type).name);
        ability_str = strings_map::get_instance()->get_ingame_string(strings_ingame_gotarmor_type_arms_msg) + " " + weapon_name + ".";
    } else if (armor_type == ARMOR_TYPE_LEGS) {
        type_str = strings_map::get_instance()->get_ingame_string(strings_ingame_gotarmor_type_legs);
        if (type == ARMOR_ABILITY_LEGS_AIRDASH) {
            ability_str = strings_map::get_instance()->get_ingame_string(strings_ingame_gotarmor_type_legs_msg1);
        } else if (type == ARMOR_ABILITY_LEGS_DOUBLEJUMP) {
            ability_str = strings_map::get_instance()->get_ingame_string(strings_ingame_gotarmor_type_legs_msg2);
        } else if (type == ARMOR_ABILITY_LEGS_SHORYUKEN) {
            ability_str = strings_map::get_instance()->get_ingame_string(strings_ingame_gotarmor_type_legs_msg3);
        }
    } else {
        type_str = strings_map::get_instance()->get_ingame_string(strings_ingame_gotarmor_type_body);
        if (type == ARMOR_ABILITY_BODY_EXTENDEDIMMUNITY) {
            ability_str = strings_map::get_instance()->get_ingame_string(strings_ingame_gotarmor_type_body_msg1);
        } else if (type == ARMOR_ABILITY_BODY_HALFDAMAGE) {
            ability_str = strings_map::get_instance()->get_ingame_string(strings_ingame_gotarmor_type_body_msg2);
        } else if (type == ARMOR_ABILITY_BODY_NOPUSHBACK) {
            ability_str = strings_map::get_instance()->get_ingame_string(strings_ingame_gotarmor_type_body_msg3);
        } else if (type == ARMOR_ABILITY_BODY_SPIKESIMMMUNE) {
            ability_str = strings_map::get_instance()->get_ingame_string(strings_ingame_gotarmor_type_body_msg4);
        }
    }

    std::string lines[] = {type_str, strings_map::get_instance()->get_ingame_string(strings_ingame_gotarmor_type_ability), ability_str};
    show_dialog("canotus_face.png", true, lines, true);
}

void dialogs::show_centered_dialog(std::vector<string> lines)
{
    draw_dialog_bg();
    draw_lib.update_screen();
    st_position dialog_pos = graphLib.get_dialog_pos();
    draw_lib.update_screen();

    /// @TODO: usar show_config_bg e hide_config_bg da graphLib - modificar para aceitar centered (que é o atual) ou top ou bottom
    for (int i=0; i<FS_DIALOG_LINES; i++) {
        std::string line = lines[i];
        graphLib.draw_text(dialog_pos.x+52, i*11+(dialog_pos.y+16), line);


        draw_lib.update_screen();
        timer.delay(50);
    }

    graphLib.show_dialog_button(1);
    draw_lib.update_screen();


    input.clean_confirm_button();
    input.wait_keypress();
    gameControl.game_unpause();
}

void dialogs::show_boss_dialog(int stage_n)
{
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    std::vector<std::string> stage_dialogs = fio_str.get_stage_dialogs(stage_n, SharedData::get_instance()->current_language, true);
    int n = 0;

    int init_pos = 30;
    int init_player_pos = init_pos + 6 + game_save.selected_player * 6;
    // p1: 36, p2: 42, p3: 48, p4: 54

    if (stage_dialogs.at(init_pos).length() <= 0) {
        return;
    }
    // BOSS SPEAKING #1
    std::string lines[FS_DIALOG_LINES];
    for (int i=init_pos; i<init_pos+FS_DIALOG_LINES; i++) {
        lines[n] = stage_dialogs.at(i);
        n++;
    }
    std::string boss_face(stage_data.dialog_face_graphics_filename);
    if (boss_face.length() <= 0) {
        boss_face = std::string("dr_kanotus.png");
    }
    show_dialog(boss_face, stage_data.dialog_top_side, lines, true);

    // PLAYER ANSWER #1
    n = 0;
    for (int i=init_player_pos; i<init_player_pos+FS_DIALOG_LINES; i++) {
        lines[n] = stage_dialogs.at(i);
        n++;
    }
    show_dialog(GameMediator::get_instance()->player_list_v3_1[game_save.selected_player].face_filename, stage_data.dialog_top_side, lines, true); /// @TODO: create "extern" for player number

    // BOSS SPEAKING #2
    int init_pos2 = 33;
    if (stage_dialogs.at(init_pos2).length() > 0) {
        n = 0;
        for (int i=init_pos2; i<init_pos2+FS_DIALOG_LINES; i++) {
            lines[n] = stage_dialogs.at(i);
            n++;
        }
        show_dialog(boss_face, stage_data.dialog_top_side, lines, true);
    }

    // PLAYER ANSWER #2
    if (stage_dialogs.at(init_player_pos+3).length() > 0) {
        n = 0;
        for (int i=init_player_pos+3; i<init_player_pos+3+FS_DIALOG_LINES; i++) {
            lines[n] = stage_dialogs.at(i);
            n++;
        }
        show_dialog(GameMediator::get_instance()->player_list_v3_1[game_save.selected_player].face_filename, stage_data.dialog_top_side, lines, true);
    }
}
