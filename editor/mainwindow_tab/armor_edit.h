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
    void on_p1_arm_comboBox_currentIndexChanged(int index);
    void on_p1_body_comboBox_currentIndexChanged(int index);
    void on_p1_legs_comboBox_currentIndexChanged(int index);
    void on_p2_arm_comboBox_currentIndexChanged(int index);
    void on_p2_body_comboBox_currentIndexChanged(int index);
    void on_p2_legs_comboBox_currentIndexChanged(int index);
    void on_p3_arm_comboBox_currentIndexChanged(int index);
    void on_p3_body_comboBox_currentIndexChanged(int index);
    void on_p3_legs_comboBox_currentIndexChanged(int index);
    void on_p4_arm_comboBox_currentIndexChanged(int index);
    void on_p4_body_comboBox_currentIndexChanged(int index);
    void on_p4_legs_comboBox_currentIndexChanged(int index);

    void on_arm_player1_toolButton1_clicked();
    void on_arm_player1_toolButton2_clicked();
    void on_arm_player1_toolButton3_clicked();
    void on_arm_player2_toolButton1_clicked();
    void on_arm_player2_toolButton2_clicked();
    void on_arm_player2_toolButton3_clicked();
    void on_arm_player3_toolButton1_clicked();
    void on_arm_player3_toolButton2_clicked();
    void on_arm_player3_toolButton3_clicked();
    void on_arm_player4_toolButton1_clicked();
    void on_arm_player4_toolButton2_clicked();
    void on_arm_player4_toolButton3_clicked();
    void on_body_player1_toolButton1_clicked();
    void on_body_player1_toolButton2_clicked();
    void on_body_player1_toolButton3_clicked();
    void on_body_player2_toolButton1_clicked();
    void on_body_player2_toolButton2_clicked();
    void on_body_player2_toolButton3_clicked();
    void on_body_player3_toolButton1_clicked();
    void on_body_player3_toolButton2_clicked();
    void on_body_player3_toolButton3_clicked();
    void on_body_player4_toolButton1_clicked();
    void on_body_player4_toolButton2_clicked();
    void on_body_player4_toolButton3_clicked();
    void on_legs_player1_toolButton1_clicked();
    void on_legs_player1_toolButton2_clicked();
    void on_legs_player1_toolButton3_clicked();
    void on_legs_player2_toolButton1_clicked();
    void on_legs_player2_toolButton2_clicked();
    void on_legs_player2_toolButton3_clicked();
    void on_legs_player3_toolButton1_clicked();
    void on_legs_player3_toolButton2_clicked();
    void on_legs_player3_toolButton3_clicked();
    void on_legs_player4_toolButton1_clicked();
    void on_legs_player4_toolButton2_clicked();
    void on_legs_player4_toolButton3_clicked();

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
