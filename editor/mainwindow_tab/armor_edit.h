#ifndef ARMOR_EDIT_H
#define ARMOR_EDIT_H

#include <QWidget>

namespace Ui {
class armor_edit;
}

class armor_edit : public QWidget
{
    Q_OBJECT

public:
    explicit armor_edit(QWidget *parent = 0);
    ~armor_edit();


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

private:
    void fill_armor_abilities();

private:
    Ui::armor_edit *ui;
    bool _data_loading;
};

#endif // ARMOR_EDIT_H
