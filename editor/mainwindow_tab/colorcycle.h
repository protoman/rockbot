#ifndef COLORCYCLE_H
#define COLORCYCLE_H

#include <QWidget>
#include <QSpinBox>

namespace Ui {
class colorcycle_edit;
}

class colorcycle_edit : public QWidget
{
	Q_OBJECT
	
public:
	explicit colorcycle_edit(QWidget *parent = 0);
	~colorcycle_edit();
    void reload();

private:
	void pick_color(short n);
	void reload_colors();

	
private slots:

	void on_stage_selector_currentIndexChanged(int index);

	void on_color_pick_button1_clicked();

	void on_color_pick_button2_clicked();

	void on_color_pick_button3_clicked();

	void on_color_pick_button4_clicked();

	void on_color_pick_button5_clicked();

	void on_color_pick_button6_clicked();

	void on_color_pick_button7_clicked();

	void on_color_pick_button8_clicked();

	void on_color_pick_button9_clicked();

	void on_color_pick_button10_clicked();

	void on_comboBox_2_currentIndexChanged(int index);

	void on_comboBox_currentIndexChanged(int index);

	void on_duration1_valueChanged(int arg1);

	void on_duration2_valueChanged(int arg1);

	void on_duration3_valueChanged(int arg1);

	void on_duration4_valueChanged(int arg1);

	void on_duration5_valueChanged(int arg1);

	void on_duration6_valueChanged(int arg1);

	void on_duration7_valueChanged(int arg1);

	void on_duration8_valueChanged(int arg1);

	void on_duration9_valueChanged(int arg1);

	void on_duration10_valueChanged(int arg1);

	void on_show_colorcycle1_checkbox_toggled(bool checked);

	void on_show_colorcycle2_checkbox_toggled(bool checked);

	void on_show_colorcycle3_checkbox_toggled(bool checked);

public slots:
	void pick_color_slot();

private:
	Ui::colorcycle_edit *ui;
	int _selected_n;
	int _selected_color_key;
	QList<QWidget*> _button_list;
	QList<QSpinBox*> _timer_list;
	bool _data_loading;
};

#endif // COLORCYCLE_H
