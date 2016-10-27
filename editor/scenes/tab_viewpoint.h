#ifndef TAB_VIEWPOINT_H
#define TAB_VIEWPOINT_H

#include <QWidget>
#include "../../defines.h"
#include "../file/fio_scenes.h"
#include "scenes/scenesmediator.h"


namespace Ui {
class tab_viewpoint;
}

class tab_viewpoint : public QWidget
{
    Q_OBJECT

public:
    explicit tab_viewpoint(QWidget *parent = 0);
    ~tab_viewpoint();
    void save_data();
    void reload();

private:
    void change_fields_enabled(bool state);
    void fill_data();

public slots:
    void change_w(int value);
    void change_h(int value);

private slots:
    void on_filename_comboBox_currentIndexChanged(const QString &arg1);
    void set_fields(int index);
    void update_preview_image(int index);

    void on_add_Button_clicked();

    void on_select_comboBox_currentIndexChanged(int index);

    void on_name_textEdit_textChanged(const QString &arg1);

    void on_init_x_spinBox_valueChanged(int arg1);

    void on_init_y_spinBox_valueChanged(int arg1);

    void on_destx_spinBox_valueChanged(int arg1);

    void on_desty_spinBox_valueChanged(int arg1);

    void on_img_area_w_valueChanged(int arg1);

    void on_img_area_h_valueChanged(int arg1);

    void on_delay_spinBox_valueChanged(int arg1);

    void on_comboBox_currentIndexChanged(int index);

    void on_posx_spinBox_valueChanged(int arg1);

    void on_posy_spinBox_valueChanged(int arg1);

    void on_pushButton_clicked();

private:
    Ui::tab_viewpoint *ui;
    CURRENT_FILE_FORMAT::fio_scenes fio;
    bool data_loading;
};

#endif // TAB_VIEWPOINT_H
