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


    void on_cutscenePre_comboBox_currentIndexChanged(int index);

    void on_cutscenePos_comboBox_currentIndexChanged(int index);

    void on_stages_tab_bossname_lineedit_textChanged(const QString &arg1);

    void on_dialogs_line1_text1_textChanged(const QString &arg1);
    void on_dialogs_line1_text2_textChanged(const QString &arg1);
    void on_dialogs_line1_text3_textChanged(const QString &arg1);
    void on_dialogs_line2_text1_textChanged(const QString &arg1);
    void on_dialogs_line2_text2_textChanged(const QString &arg1);
    void on_dialogs_line2_text3_textChanged(const QString &arg1);
    void on_dialogs_answer1_text1_textChanged(const QString &arg1);
    void on_dialogs_answer1_text2_textChanged(const QString &arg1);
    void on_dialogs_answer1_text3_textChanged(const QString &arg1);
    void on_dialogs_answer2_text1_textChanged(const QString &arg1);
    void on_dialogs_answer2_text2_textChanged(const QString &arg1);
    void on_dialogs_answer2_text3_textChanged(const QString &arg1);
    void on_boss_dialog_text1_line1_textChanged(const QString &arg1);

    void on_boss_dialog_text1_line2_textChanged(const QString &arg1);

    void on_boss_dialog_text1_line3_textChanged(const QString &arg1);

    void on_boss_dialog_text2_line1_textChanged(const QString &arg1);

    void on_boss_dialog_text2_line2_textChanged(const QString &arg1);

    void on_boss_dialog_text2_line3_textChanged(const QString &arg1);

    void on_boss_dialog_answer1_line1_textChanged(const QString &arg1);

    void on_boss_dialog_answer1_line2_textChanged(const QString &arg1);

    void on_boss_dialog_answer1_line3_textChanged(const QString &arg1);

    void on_boss_dialog_answer2_line1_textChanged(const QString &arg1);

    void on_boss_dialog_answer2_line2_textChanged(const QString &arg1);

    void on_boss_dialog_answer2_line3_textChanged(const QString &arg1);

    void on_language_comboBox_currentIndexChanged(int index);

    void on_checkBox_stateChanged(int arg1);

public slots:
    void on_string_selected(int string_id);

private:
    void fill_stage_tab_data(int language_n);
    void update_stage_data(int language_n);
    void string_tooltip_click(int *property, QLineEdit* qline);

private:
	Ui::stage_edit *ui;
    bool _data_loading;
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    std::map<int, std::vector<std::string> > stage_dialog_default_language_list;
    bool initialized;
};

#endif // STAGE_EDIT_H
