#ifndef GAMESCENES_H
#define GAMESCENES_H

#include <QWidget>

namespace Ui {
class GameScenes;
}

class GameScenes : public QWidget
{
    Q_OBJECT

public:
    explicit GameScenes(QWidget *parent = 0);
    ~GameScenes();

private:
    Ui::GameScenes *ui;
};

#endif // GAMESCENES_H
