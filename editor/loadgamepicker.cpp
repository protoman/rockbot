#include "loadgamepicker.h"
#include "ui_loadgamepicker.h"
#include <mediator.h>
#include <stdio.h>
#include <vector>

#include "file/file_io.h"
#include "mainwindow.h"

extern std::string FILEPATH;
extern std::string GAMEPATH;

loadGamePicker::loadGamePicker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::loadGamePicker)
{
	ui->setupUi(this);

    CURRENT_FILE_FORMAT::file_io fio;
    std::vector<std::string> game_list = fio.read_game_list();

    for (int i=0; i<game_list.size(); i++) {
        ui->gameList->addItem(QString(game_list.at(i).c_str()));
    }


}

loadGamePicker::~loadGamePicker()
{
    delete ui;
}

void loadGamePicker::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void loadGamePicker::on_buttonBox_accepted()
{
    std::cout << ">>>>>>>>>> ui->gameList->currentRow(): " << ui->gameList->currentRow() << std::endl;

    if (ui->gameList->currentRow() == 0) {
        FILEPATH = GAMEPATH + std::string("/data/");
    } else {
        FILEPATH = GAMEPATH + std::string("/games/") + ui->gameList->currentItem()->text().toStdString() + std::string("/");
    }
    Mediator::get_instance()->loadGame();
    emit game_picked();

	//printf("DEBUG - loadGamePicker::on_buttonBox_accepted - selectedItem: %d\n", ui->gameList->currentRow());
}
