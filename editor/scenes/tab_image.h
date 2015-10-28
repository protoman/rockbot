#ifndef TAB_IMAGE_H
#define TAB_IMAGE_H

#include <QWidget>
#include "../../defines.h"
#include "../file/fio_scenes.h"
#include "scenes/scenesmediator.h"

namespace Ui {
class tab_image;
}

class tab_image : public QWidget
{
    Q_OBJECT

public:
    explicit tab_image(QWidget *parent = 0);
    ~tab_image();
    void save_data();
    void reload();


private slots:
    void set_fields(int index);
    void update_preview_image(int index);
    void on_add_Button_clicked();

    void on_select_comboBox_currentIndexChanged(int index);

    void on_destx_spinBox_valueChanged(int arg1);

    void on_desty_spinBox_valueChanged(int arg1);

    void on_delay_spinBox_valueChanged(int arg1);

    void on_blocking_checkBox_toggled(bool checked);

    void on_filename_comboBox_currentIndexChanged(const QString &arg1);

    void on_init_y_spinBox_valueChanged(int arg1);

    void on_init_x_spinBox_valueChanged(int arg1);

    void on_img_area_x_valueChanged(int arg1);

    void on_img_area_y_valueChanged(int arg1);

    void on_img_area_w_valueChanged(int arg1);

    void on_img_area_h_valueChanged(int arg1);

    void change_w(int value);
    void change_h(int value);

    void on_comboBox_currentIndexChanged(int index);

    void on_name_textEdit_textChanged(const QString &arg1);

private:
    void change_fields_enabled(bool state);
    void fill_data();

private:
    Ui::tab_image *ui;
    CURRENT_FILE_FORMAT::fio_scenes fio;
    bool data_loading;
};

#endif // TAB_IMAGE_H
