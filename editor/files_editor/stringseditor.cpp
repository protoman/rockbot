#include "stringseditor.h"
#include "ui_stringseditor.h"

StringsEditor::StringsEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StringsEditor)
{
    ui->setupUi(this);
}

StringsEditor::~StringsEditor()
{
    delete ui;
}
