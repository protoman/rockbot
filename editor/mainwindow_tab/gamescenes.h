#ifndef GAMESCENES_H
#define GAMESCENES_H

#include <QWidget>
#include "file/fio_scenes.h"
#include "scenes/scenesmediator.h"
#include "mediator.h"

namespace Ui {
class GameScenes;
}

class GameScenes : public QWidget
{
    Q_OBJECT

public:
    explicit GameScenes(QWidget *parent = 0);
    ~GameScenes();
    void reload();

private:
    void fill_data();

private slots:
    void on_company_combo_currentIndexChanged(int index);
    void on_intro_combo_currentIndexChanged(int index);
    void on_intro_beaten_combo_currentIndexChanged(int index);
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
