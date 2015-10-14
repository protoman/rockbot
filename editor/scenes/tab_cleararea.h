#ifndef TAB_CLEARAREA_H
#define TAB_CLEARAREA_H

#include <QWidget>

#include "../../defines.h"
#include "../file/fio_scenes.h"
#include "scenes/scenesmediator.h"

namespace Ui {
class TabClearArea;
}

class TabClearArea : public QWidget
{
    Q_OBJECT

public:
    explicit TabClearArea(QWidget *parent = 0);
    ~TabClearArea();
    void save_data();

private:
    void change_fields_enabled(bool state);
    void fill_data();
    void set_fields(int index);
    void reload_button_color(int index);

private slots:
    void on_add_pushButton_clicked();

    void on_select_comboBox_currentIndexChanged(int index);

    void on_color_pushButton_clicked();

    void on_name_lineEdit_textChanged(const QString &arg1);

    void on_x_spinBox_valueChanged(int arg1);

    void on_y_spinBox_valueChanged(int arg1);

    void on_w_spinBox_valueChanged(int arg1);

    void on_h_spinBox_valueChanged(int arg1);

public slots:
    void pick_color_slot(QColor color);


private:
    Ui::TabClearArea *ui;
    CURRENT_FILE_FORMAT::fio_scenes fio;
    bool data_loading;
};

#endif // TAB_CLEARAREA_H
