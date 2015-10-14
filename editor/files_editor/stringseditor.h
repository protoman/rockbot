#ifndef STRINGSEDITOR_H
#define STRINGSEDITOR_H

#include "file/format.h"
#include "file/fio_strings.h"

#include <QDialog>
#include <QSignalMapper>

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

public slots:
    void on_text_changed(int n);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::StringsEditor *ui;
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    QSignalMapper signal_mapper;
    std::vector<std::string> string_list;
};

#endif // STRINGSEDITOR_H
