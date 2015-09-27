#include "loadgamepicker.h"
#include "ui_loadgamepicker.h"
#include <mediator.h>
#include <stdio.h>

loadGamePicker::loadGamePicker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::loadGamePicker)
{
	int n, i;
		char temp_name[512];

	ui->setupUi(this);
	char tempStr[512];
	/*
    sprintf(tempStr, "%s/game", FILEPATH.c_str());
    n = Mediator::get_instance()->freeSlot(tempStr, "gme");
	printf("DEBUG.loadGamePicker::loadGamePicker - n: %d\n", n);
	for (i=1; i<n; i++) {
        Mediator::get_instance()->getGameName(i);
        printf("DEBUG - adding game: '%s'\n", Mediator::get_instance()->gameName);
        sprintf(temp_name, "%s - %s", Mediator::get_instance()->gameName, Mediator::get_instance()->centNumber);
		ui->gameList->addItem(temp_name);
	}
	*/
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
	int n = ui->gameList->currentRow()+1;
    Mediator::get_instance()->loadGame(n);

	//printf("DEBUG - loadGamePicker::on_buttonBox_accepted - selectedItem: %d\n", ui->gameList->currentRow());
}
