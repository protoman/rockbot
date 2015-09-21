#ifndef TEXT_H
#define TEXT_H

#include <QDialog>
#include "scenes/scenesmediator.h"
#include "../../defines.h"
#include "../file/v3/3_0_1/file_scene.h"
#include "../file/fio_scenes.h"


namespace Ui {
class TabText;
}

class TabText : public QDialog
{
    Q_OBJECT

public:
    explicit TabText(QWidget *parent = 0);
    ~TabText();
    void save_data();


private slots:
    void on_AddButton_clicked();

    void on_positionType_currentIndexChanged(int index);

    void on_lineEdit1_textChanged(const QString &arg1);

    void on_lineEdit2_textChanged(const QString &arg1);

    void on_lineEdit3_textChanged(const QString &arg1);

    void on_lineEdit4_textChanged(const QString &arg1);

    void on_lineEdit5_textChanged(const QString &arg1);

    void on_lineEdit6_textChanged(const QString &arg1);

    void on_select_comboBox_currentIndexChanged(int index);

private:
    void change_fields_enabled(bool state);
    void change_x_y_fields_enabled();
    void fill_data();
    void set_fields(int index);

private:
    Ui::TabText *ui;
    CURRENT_FILE_FORMAT::fio_scenes fio;
    bool data_loading;
    ScenesMediator *mediator;
};

#endif // TEXT_H
