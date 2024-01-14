#ifndef TAB_PARALLAX_H
#define TAB_PARALLAX_H

#include <QWidget>
#include <QProcess>

#include "../../defines.h"
#include "../file/fio_scenes.h"
#include "scenes/scenesmediator.h"


namespace Ui {
class tab_parallax;
}

class tab_parallax : public QWidget
{
    Q_OBJECT

public:
    explicit tab_parallax(QWidget *parent = nullptr);
    ~tab_parallax();
    void save_data();
    void reload();


private:
    void fill_data();
    void set_fields(int index);
    void set_layer_fields(int layer_n);
    void set_all_layer_images(int parallax_n);
    void update_preview_image(int parallax_n, int layer_n);

private slots:
    void on_parallax_select_comboBox_currentIndexChanged(int index);

    void on_pushButton_clicked();

    void on_parallax_layer_comboBox_currentIndexChanged(int index);

    void on_parallax_layer_image_comboBox_currentIndexChanged(const QString &arg1);

    void on_parallax_layer_ypos_spinBox_valueChanged(int arg1);

    void on_parallax_layer_maxh_spinBox_valueChanged(int arg1);

    void on_parallax_layer_speed_spinBox_valueChanged(int arg1);

    void on_run_pushButton_clicked();

    void on_parallax_delay_spinBox_valueChanged(int arg1);

    void on_duration_spinBox_valueChanged(int arg1);

private:
    Ui::tab_parallax *ui;
    CURRENT_FILE_FORMAT::fio_scenes fio;
    bool data_loading;
    QProcess process;
};

#endif // TAB_PARALLAX_H
