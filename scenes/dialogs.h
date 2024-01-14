#ifndef DIALOGS_H
#define DIALOGS_H

#include <string>
#include <map>
#include <vector>
#include "../defines.h"
#include "../file/format/st_common.h"
#include "../file/format.h"

class dialogs
{
public:
    dialogs();
    void show_dialog(std::string face_file, bool is_left, std::string lines[3], bool show_btn);

    bool show_leave_game_dialog();

    bool show_yes_no_dialog(std::string lines[3]);

    void show_timed_dialog(std::string face_file, bool is_left, std::string lines[3], short int timer, bool show_btn);

    void showGotArmorDialog(e_ARMOR_PIECES armor_type);

    void show_centered_dialog(std::vector<std::string> lines);

    void show_boss_dialog(int stage_n);

private:
    void draw_dialog_bg();

private:
    bool is_showing_dialog_bg; /**< TODO */
};

#endif // DIALOGS_H
