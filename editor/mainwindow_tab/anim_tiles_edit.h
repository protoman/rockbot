#ifndef ANIM_TILES_EDIT_H
#define ANIM_TILES_EDIT_H

#include <QWidget>
#include "mainwindow_tab/anim/animpackimport.h"

namespace Ui {
class anim_tiles_edit;
}

class anim_tiles_edit : public QWidget
{
    Q_OBJECT

public:
    explicit anim_tiles_edit(QWidget *parent = 0);
    ~anim_tiles_edit();
    void reload();

private:
    void fill_data();
    void set_data(int index);

public slots:
    void onChangeFrame();

private slots:
    void on_addBlock_button_clicked();
    void on_graphic_combo_currentIndexChanged(const QString &arg1);
    void on_frameDelay_spinBox_valueChanged(int arg1);

    void on_currentBlock_combo_currentIndexChanged(int index);

    void on_pushButton_clicked();

private:
    Ui::anim_tiles_edit *ui;
    bool data_loading;
};

#endif // ANIM_TILES_EDIT_H
