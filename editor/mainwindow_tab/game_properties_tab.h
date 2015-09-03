#ifndef GAME_PROPERTIES_TAB_H
#define GAME_PROPERTIES_TAB_H

#include <QWidget>

namespace Ui {
class game_properties_tab;
}

class game_properties_tab : public QWidget
{
    Q_OBJECT
    
public:
    explicit game_properties_tab(QWidget *parent = 0);
    ~game_properties_tab();
    void reload();


private slots:
    void on_special_item1_combo_currentIndexChanged(int index);

    void on_special_item2_combo_currentIndexChanged(int index);

    void on_semicharged_shot_combo_currentIndexChanged(int index);

    void on_stagefaces_stage_combo_currentIndexChanged(int index);

    void on_stagefaces_face_combo_currentIndexChanged(const QString &arg1);

    void on_lineEdit_textChanged(const QString &arg1);

private:
    void fill_data();
    
private:
    Ui::game_properties_tab *ui;
    bool _data_loading;
    int _current_stage;
};

#endif // GAME_PROPERTIES_TAB_H
