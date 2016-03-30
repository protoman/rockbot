#ifndef ARTIFICIAL_INTELIGENCE_TAB_H
#define ARTIFICIAL_INTELIGENCE_TAB_H

#include <QWidget>
#include <QComboBox>

namespace Ui {
class artificial_inteligence_tab;
}

class artificial_inteligence_tab : public QWidget
{
    Q_OBJECT
    
public:
    explicit artificial_inteligence_tab(QWidget *parent = 0);
    ~artificial_inteligence_tab();
    void reload();
    
private slots:
	void on_ai_selector_currentIndexChanged(int index);
	void fill_data(int index);
	void change_action(int index, int action_n);

	void on_action1_currentIndexChanged(int index);

	void on_chance1_valueChanged(int arg1);

	void on_chance2_valueChanged(int arg1);

	void on_chance3_valueChanged(int arg1);

	void on_chance4_valueChanged(int arg1);

	void on_action2_currentIndexChanged(int index);

	void on_action3_currentIndexChanged(int index);

	void on_action4_currentIndexChanged(int index);

	void on_parameter1_currentIndexChanged(int index);

	void on_parameter2_currentIndexChanged(int index);

	void on_parameter3_currentIndexChanged(int index);

	void on_parameter4_currentIndexChanged(int index);

	void on_next1_currentIndexChanged(int index);

	void on_next2_currentIndexChanged(int index);

	void on_next3_currentIndexChanged(int index);

	void on_next4_currentIndexChanged(int index);

	void on_delay1_valueChanged(int arg1);

	void on_delay2_valueChanged(int arg1);

	void on_delay3_valueChanged(int arg1);

	void on_delay4_valueChanged(int arg1);

    void on_action5_currentIndexChanged(int index);

    void on_action6_currentIndexChanged(int index);

    void on_action7_currentIndexChanged(int index);

    void on_action8_currentIndexChanged(int index);

    void on_parameter5_currentIndexChanged(int index);

    void on_parameter6_currentIndexChanged(int index);

    void on_parameter7_currentIndexChanged(int index);

    void on_parameter8_currentIndexChanged(int index);

    void on_next5_currentIndexChanged(int index);

    void on_next6_currentIndexChanged(int index);

    void on_next7_currentIndexChanged(int index);

    void on_next8_currentIndexChanged(int index);

    void on_delay5_valueChanged(int arg1);

    void on_delay6_valueChanged(int arg1);

    void on_delay7_valueChanged(int arg1);

    void on_delay8_valueChanged(int arg1);

    void on_chance5_valueChanged(int arg1);

    void on_chance6_valueChanged(int arg1);

    void on_chance7_valueChanged(int arg1);

    void on_chance8_valueChanged(int arg1);

    void on_near_action_currentIndexChanged(int index);

    void on_near_extra_parameter_currentIndexChanged(int index);

    void on_hit_extra_parameter_currentIndexChanged(int index);

    void on_dead_extra_parameter_currentIndexChanged(int index);

    void on_hit_action_currentIndexChanged(int index);

    void on_dead_action_currentIndexChanged(int index);

private:
    void fill_next_combos();
    void fill_next_combo(QComboBox *combo);

private:
    Ui::artificial_inteligence_tab *ui;
	bool _filling_data;
};

#endif // ARTIFICIAL_INTELIGENCE_TAB_H
