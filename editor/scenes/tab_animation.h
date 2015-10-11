#ifndef TAB_ANIMATION_H
#define TAB_ANIMATION_H

#include <QWidget>

#include "../../defines.h"
#include "../file/fio_scenes.h"
#include "scenes/scenesmediator.h"


namespace Ui {
class TabAnimation;
}

class TabAnimation : public QWidget
{
    Q_OBJECT

public:
    explicit TabAnimation(QWidget *parent = 0);
    ~TabAnimation();
    void save_data();

private slots:
    void on_add_pushButton_clicked();
    void on_select_comboBox_currentIndexChanged(int index);
    void on_name_lineEdit_textChanged(const QString &arg1);
    void on_filename_comboBox_currentIndexChanged(const QString &arg1);
    void on_width_spinBox_valueChanged(int arg1);
    void on_height_spinBox_valueChanged(int arg1);
    void on_duration_spinBox_valueChanged(int arg1);
    void on_posx_spinBox_valueChanged(int arg1);
    void on_posy_spinBox_valueChanged(int arg1);

private:
    void change_fields_enabled(bool state);
    void fill_data();
    void set_fields(int index);

public slots:
    void change_w(int value);
    void change_h(int value);

private:
    Ui::TabAnimation *ui;
    CURRENT_FILE_FORMAT::fio_scenes fio;
    bool data_loading;
};

#endif // TAB_ANIMATION_H
