#ifndef STRINGSEDITOR_H
#define STRINGSEDITOR_H

#include <QDialog>

namespace Ui {
class StringsEditor;
}

class StringsEditor : public QDialog
{
    Q_OBJECT

public:
    explicit StringsEditor(QWidget *parent = 0);
    ~StringsEditor();

private:
    Ui::StringsEditor *ui;
};

#endif // STRINGSEDITOR_H
