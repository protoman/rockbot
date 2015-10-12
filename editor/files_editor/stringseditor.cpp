#include "stringseditor.h"
#include "ui_stringseditor.h"

#include <QLineEdit>
#include <QSpacerItem>

StringsEditor::StringsEditor(QWidget *parent) : QDialog(parent), ui(new Ui::StringsEditor)
{
    std::vector<std::string> string_list = fio_str.load_game_strings();
    ui->setupUi(this);

    connect(signal_mapper, SIGNAL(mapped(const int &)), this, SLOT(onButtonClicked(const int &)));

    for (int i=0; i<string_list.size(); i++) {
        QLineEdit* line_editor;
        line_editor = new QLineEdit();
        line_editor->setText(QString(string_list.at(i).c_str()));
        ui->tabGame_gridLayout->addWidget(line_editor, i, 0);
        signal_mapper->setMapping(line_editor, i);
    }
}

StringsEditor::~StringsEditor()
{
    delete ui;
}

void StringsEditor::on_text_changed(int n, QString value)
{

}
