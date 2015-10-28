#ifndef GAMESCENES_H
#define GAMESCENES_H

#include <QWidget>
#include "file/fio_scenes.h"
#include "scenes/scenesmediator.h"

namespace Ui {
class GameScenes;
}

class GameScenes : public QWidget
{
    Q_OBJECT

public:
    explicit GameScenes(QWidget *parent = 0);
    ~GameScenes();

private slots:
    void on_company_combo_currentIndexChanged(int index);
    void on_intro_combo_currentIndexChanged(int index);
    void on_intro_beaten_combo_currentIndexChanged(int index);
    void on_pre_castle1_combo_currentIndexChanged(int index);
    void on_castle1_stage1_combo_currentIndexChanged(int index);
    void on_castle1_stage2_combo_currentIndexChanged(int index);
    void on_castle1_stage3_combo_currentIndexChanged(int index);
    void on_castle1_stage4_combo_currentIndexChanged(int index);
    void on_castle1_stage5_combo_currentIndexChanged(int index);
    void on_pre_castle2_combo_currentIndexChanged(int index);
    void on_castle2_stage1_combo_currentIndexChanged(int index);
    void on_castle2_stage2_combo_currentIndexChanged(int index);
    void on_castle2_stage3_combo_currentIndexChanged(int index);
    void on_castle2_stage4_combo_currentIndexChanged(int index);
    void on_castle2_stage5_combo_currentIndexChanged(int index);
    void on_ending_player1_combo_currentIndexChanged(int index);
    void on_ending_player2_combo_currentIndexChanged(int index);
    void on_ending_player3_combo_currentIndexChanged(int index);
    void on_ending_player4_combo_currentIndexChanged(int index);
    void on_ending_credits_combo_currentIndexChanged(int index);

private:
    Ui::GameScenes *ui;
    bool data_loading;
    CURRENT_FILE_FORMAT::fio_scenes fio_scn;
};

#endif // GAMESCENES_H
