#ifndef TAB_CHANGECOLORCYCLE_H
#define TAB_CHANGECOLORCYCLE_H

#include <QWidget>

#include "../../defines.h"
#include "../file/v3/3_0_1/file_scene.h"
#include "../file/fio_scenes.h"
#include "scenes/scenesmediator.h"

namespace Ui {
class TabChangeColorcycle;
}

class TabChangeColorcycle : public QWidget
{
    Q_OBJECT

public:
    explicit TabChangeColorcycle(QWidget *parent = 0);
    ~TabChangeColorcycle();
    void save_data();

private slots:
    void on_select_comboBox_currentIndexChanged(int index);

    void on_name_lineEdit_textChanged(const QString &arg1);

    void on_colorcycle_comboBox_currentIndexChanged(int index);

    void on_add_pushButton_clicked();

private:
    void change_fields_enabled(bool state);
    void fill_data();
    void set_fields(int index);

private:
    Ui::TabChangeColorcycle *ui;
    CURRENT_FILE_FORMAT::fio_scenes fio;
    bool data_loading;
};

#endif // TAB_CHANGECOLORCYCLE_H
