#ifndef STRINGSEDITOR_H
#define STRINGSEDITOR_H

#include "file/format.h"
#include "file/fio_strings.h"
#include "file/file_io.h"
#include "model/stringeditmodel.h"

#include <QDialog>
#include <QSignalMapper>
#include <QLineEdit>
#include <QSpacerItem>
#include <QSignalMapper>
#include <QComboBox>
#include <QLabel>
#include <QDir>

namespace Ui {
class StringsEditor;
}

class StringsEditor : public QDialog
{
    Q_OBJECT

public:
    explicit StringsEditor(QWidget *parent = 0);
    ~StringsEditor();
    void save_data();

private:
    void fill_data();
    void fill_translation();
    void load_language(std::string filename);

public slots:
    void on_text_changed(int n);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_addLanguage_pushButton_clicked();

    void on_languageName_lineEdit_textChanged(const QString &arg1);

    void on_languageSelector_comboBox_currentTextChanged(const QString &arg1);

private:
    Ui::StringsEditor *ui;
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    CURRENT_FILE_FORMAT::file_io fio;
    QSignalMapper signal_mapper;
    std::vector<std::string> string_list;
    std::vector<std::string> translation_string_list;
    std::vector<QLineEdit*> editor_line_list;
    bool data_loading;
    std::string current_lang;
    StringEditModel string_edit_model;
};

#endif // STRINGSEDITOR_H
