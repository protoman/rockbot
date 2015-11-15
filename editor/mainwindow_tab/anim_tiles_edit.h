#ifndef ANIM_TILES_EDIT_H
#define ANIM_TILES_EDIT_H

#include <QWidget>

namespace Ui {
class anim_tiles_edit;
}

class anim_tiles_edit : public QWidget
{
    Q_OBJECT

public:
    explicit anim_tiles_edit(QWidget *parent = 0);
    ~anim_tiles_edit();

private:
    Ui::anim_tiles_edit *ui;
};

#endif // ANIM_TILES_EDIT_H
