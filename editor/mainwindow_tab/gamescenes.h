#ifndef GAMESCENES_H
#define GAMESCENES_H

#include <QWidget>
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
    void on_company_scene_currentIndexChanged(int index);

private:
    Ui::GameScenes *ui;
    bool data_loading;
};

#endif // GAMESCENES_H
