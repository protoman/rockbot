#ifndef TEXT_H
#define TEXT_H


#include <QDialog>
#include <QLineEdit>
#include "scenes/scenesmediator.h"
#include "defines.h"
#include "file/fio_scenes.h"
#include "file/fio_strings.h"
#include "files_editor/stringseditor.h"

namespace Ui {
class TabText;
}

class TabText : public QDialog
{
    Q_OBJECT

public:
    explicit TabText(QWidget *parent = 0);
    ~TabText();
    void save_data();
    void save_data(int n);
    void reload();


private slots:
    void on_AddButton_clicked();
    void on_positionType_currentIndexChanged(int index);
    void on_select_comboBox_currentIndexChanged(int index);
    void on_name_lineEdit_textChanged(const QString &arg1);
    void on_textX_spinBox_valueChanged(int arg1);
    void on_textY_spinBox_valueChanged(int arg1);

    void on_lineEdit1_textChanged(const QString &arg1);

    void on_lineEdit2_textChanged(const QString &arg1);

    void on_lineEdit3_textChanged(const QString &arg1);

    void on_lineEdit4_textChanged(const QString &arg1);

    void on_lineEdit5_textChanged(const QString &arg1);

    void on_lineEdit6_textChanged(const QString &arg1);

    void on_languageComboBox_currentIndexChanged(int index);

public slots:


private:
    void change_fields_enabled(bool state);
    void change_x_y_fields_enabled();
    void fill_data();
    void set_fields(int index);

private:
    Ui::TabText *ui;
    CURRENT_FILE_FORMAT::fio_scenes fio_scenes;
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    CURRENT_FILE_FORMAT::file_io fio;
    bool data_loading;
    std::string scene_text_list[SCENE_TEXT_LINES_N];
    int currentIndex;
    int currentLanguage;
    bool dataLoading;
};

#endif // TEXT_H
