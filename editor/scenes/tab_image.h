#ifndef TAB_IMAGE_H
#define TAB_IMAGE_H

#include <QWidget>
#include "../../defines.h"
#include "../file/v3/3_0_1/file_scene.h"
#include "../file/fio_scenes.h"

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


private slots:
    void on_add_Button_clicked();

    void on_select_comboBox_currentIndexChanged(int index);

    void on_x_spinBox_valueChanged(int arg1);

    void on_y_spinBox_valueChanged(int arg1);

    void on_destx_spinBox_valueChanged(int arg1);

    void on_desty_spinBox_valueChanged(int arg1);

    void on_delay_spinBox_valueChanged(int arg1);

    void on_blocking_checkBox_toggled(bool checked);

    void on_filename_comboBox_currentIndexChanged(const QString &arg1);

private:
    void change_fields_enabled(bool state);
    void fill_data();

private:
    Ui::tab_image *ui;
    CURRENT_FILE_FORMAT::fio_scenes fio;
    std::vector<CURRENT_FILE_FORMAT::file_scene_show_image> list;
    bool data_loading;
};

#endif // TAB_IMAGE_H
