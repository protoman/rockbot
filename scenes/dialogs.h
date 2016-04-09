#ifndef DIALOGS_H
#define DIALOGS_H

#include <string>
#include <map>
#include <vector>
#include "defines.h"
#include "file/format/st_common.h"
#include "file/format.h"

/**
 * @brief
 *
 */
class dialogs
{
public:
/**
 * @brief
 *
 */
    dialogs();
    /**
     * @brief
     *
     * @param stage
     */
    void show_stage_dialog(int stage_n);
    /**
     * @brief
     *
     * @param stage
     */
    void show_boss_dialog(int stage_n);
    /**
     * @brief
     *
     * @param face_file
     * @param is_left
     * @param lines[]
     * @param show_btn
     */
    void show_dialog(std::string face_file, bool is_left, std::string lines[3], bool show_btn);

    bool show_leave_game_dialog() const;

    /**
     * @brief
     *
     * @param face_file
     * @param is_left
     * @param lines[]
     * @param timer
     * @param show_btn
     */
    void show_timed_dialog(std::string face_file, bool is_left, std::string lines[3], short int timer, bool show_btn);

    void showGotArmorDialog(e_ARMOR_PIECES armor_type);

private:
    /**
     * @brief
     *
     * @param show_btn
     */
    void draw_dialog_bg();

private:
    bool is_showing_dialog_bg; /**< TODO */
};

#endif // DIALOGS_H
