#ifndef TAB_SFX_H
#define TAB_SFX_H

#include <QWidget>

#include "../../defines.h"
#include "../file/fio_scenes.h"
#include "scenes/scenesmediator.h"


namespace Ui {
class TabSfx;
}

class TabSfx : public QWidget
{
    Q_OBJECT

public:
    explicit TabSfx(QWidget *parent = 0);
    ~TabSfx();
    void save_data();
    void reload();

private:
    void change_fields_enabled(bool state);
    void fill_data();
    void set_fields(int index);


private slots:
    void on_add_pushButton_clicked();

    void on_select_comboBox_currentIndexChanged(int index);

    void on_name_lineEdit_textChanged(const QString &arg1);

    void on_filename_comboBox_currentIndexChanged(const QString &arg1);

private:
    Ui::TabSfx *ui;
    CURRENT_FILE_FORMAT::fio_scenes fio;
    bool data_loading;

};

#endif // TAB_SFX_H
