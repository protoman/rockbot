#ifndef SCENESLIST_H
#define SCENESLIST_H

#include <QDialog>

namespace Ui {
class ScenesList;
}

class ScenesList : public QDialog
{
    Q_OBJECT

public:
    explicit ScenesList(QWidget *parent = 0);
    ~ScenesList();

private:
    Ui::ScenesList *ui;
};

#endif // SCENESLIST_H
