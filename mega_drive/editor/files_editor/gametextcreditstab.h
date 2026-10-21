#ifndef GAMETEXTCREDITSTAB_H
#define GAMETEXTCREDITSTAB_H

#include <QWidget>

namespace Ui {
class GameTextCreditsTab;
}

class GameTextCreditsTab : public QWidget
{
    Q_OBJECT

public:
    explicit GameTextCreditsTab(QWidget *parent = 0);
    ~GameTextCreditsTab();
    void save_data();

private slots:
    void on_bossCredits_lineEdit1_textChanged(const QString &arg1);
    void on_bossCredits_lineEdit2_textChanged(const QString &arg1);
    void on_bossCredits_lineEdit3_textChanged(const QString &arg1);
    void on_stageSelect_comboBox_currentIndexChanged(int index);

private:
    void load_data();
    void set_stage_data(int stage_n);



private:
    Ui::GameTextCreditsTab *ui;
    int current_stage;
    bool data_loading;
    std::vector<std::string> boss_credits_data;
    std::vector<std::string> game_credits_data;
};

#endif // GAMETEXTCREDITSTAB_H
