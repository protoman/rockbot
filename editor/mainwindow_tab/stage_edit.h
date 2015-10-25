#ifndef STAGE_EDIT_H
#define STAGE_EDIT_H

#include <QWidget>
#include <QComboBox>
#include <QListWidgetItem>

#include "files_editor/stringseditor.h"

namespace Ui {
class stage_edit;
}

class stage_edit : public QWidget
{
	Q_OBJECT
	
public:
	explicit stage_edit(QWidget *parent = 0);
	~stage_edit();
    void reload();

	
private slots:
	void on_stages_tab_stage_combo_currentIndexChanged(int index);
	void on_dialogs_answer1_player_currentIndexChanged(int index);
    void on_stages_tab_bgmusic_combo_currentIndexChanged(const QString &arg1);
    void on_stages_tab_stage_name_lineedit_textChanged(const QString &arg1);
    void on_dialogs_line1_face_combo_currentIndexChanged(const QString &arg1);
    void on_personName_toolButton_clicked();

    void on_phrase1_1_toolButton_clicked();

    void on_phrase1_2_toolButton_clicked();

    void on_phrase1_3_toolButton_clicked();

    void on_phrase2_1_toolButton_clicked();

    void on_phrase2_2_toolButton_clicked();

    void on_phrase2_3_toolButton_clicked();

    void on_answer1_1_toolButton_clicked();

    void on_answer1_2_toolButton_clicked();

    void on_answer1_3_toolButton_clicked();

    void on_answer2_1_toolButton_clicked();

    void on_answer2_2_toolButton_clicked();

    void on_answer2_3_toolButton_clicked();

    void on_boss_phrase1_1_toolButton_clicked();

    void on_boss_phrase1_2_toolButton_clicked();

    void on_boss_phrase1_3_toolButton_clicked();

    void on_boss_phrase2_1_toolButton_clicked();

    void on_boss_phrase2_2_toolButton_clicked();

    void on_boss_phrase2_3_toolButton_clicked();

    void on_boss_answer1_1_toolButton_clicked();

    void on_boss_answer1_2_toolButton_clicked();

    void on_boss_answer1_3_toolButton_clicked();

    void on_boss_answer2_1_toolButton_clicked();

    void on_boss_answer2_2_toolButton_clicked();

    void on_boss_answer2_3_toolButton_clicked();

public slots:
    void on_string_selected(int string_id);

private:
	void fill_stage_tab_data();
    void update_stage_data();
    void string_tooltip_click(Sint8 *property, QLineEdit* qline);

private:
	Ui::stage_edit *ui;
    bool _data_loading;
    CURRENT_FILE_FORMAT::fio_strings fio_str;

};

#endif // STAGE_EDIT_H
