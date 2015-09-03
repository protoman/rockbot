#include "tab_text.h"
#include "ui_tab_text.h"
#include "../file/format.h"

extern std::vector<CURRENT_FILE_FORMAT::file_scene_sequence> sequences;
extern std::vector<CURRENT_FILE_FORMAT::file_scene> scenes;

TabText::TabText(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TabText)
{
    ui->setupUi(this);
}

TabText::~TabText()
{
    delete ui;
}

void TabText::fill_list()
{
    int n = 0;
    for (int i=0; i<scenes.size(); i++) {
        CURRENT_FILE_FORMAT::file_scene temp = scenes.at(i);
        if (temp.type == CURRENT_FILE_FORMAT::scenetype_show_text) {
            ui->listCombo->addItem(QString("[") + QString::number(i) + QString("]"));
            n++;
        }

    }
}
