#ifndef ARMOR_EDIT_H
#define ARMOR_EDIT_H

#include <QWidget>
#include <QLineEdit>

#include "ui_armor_edit.h"
#include "file/format.h"
#include "mediator.h"
#include "files_editor/stringseditor.h"
#include "file/fio_strings.h"


namespace Ui {
class armor_edit;
}

class armor_edit : public QWidget
{
    Q_OBJECT

public:
    explicit armor_edit(QWidget *parent = 0);
    ~armor_edit();
    void reload();


private slots:
    void on_arm_comboBox_currentIndexChanged(int index);
    void on_body_comboBox_currentIndexChanged(int index);
    void on_legs_comboBox_currentIndexChanged(int index);
    void on_playerSelect_comboBox_currentIndexChanged(int index);

public slots:
    void on_string_selected(int string_id);

private:
    void fill_armor_abilities();
    void string_tooltip_click(int *property, QLineEdit* qline);

private:
    Ui::armor_edit *ui;
    bool _data_loading;
    CURRENT_FILE_FORMAT::fio_strings fio_str;
};

#endif // ARMOR_EDIT_H
