#include "loadgamepicker.h"
#include "ui_loadgamepicker.h"
#include <mediator.h>
#include <stdio.h>

extern Mediator *dataExchanger;
extern char EDITOR_FILEPATH[512];

loadGamePicker::loadGamePicker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::loadGamePicker)
{
	int n, i;
		char temp_name[512];

	ui->setupUi(this);
	char tempStr[512];
	/*
	sprintf(tempStr, "%s/data/game", EDITOR_FILEPATH);
	n = dataExchanger->freeSlot(tempStr, "gme");
	printf("DEBUG.loadGamePicker::loadGamePicker - n: %d\n", n);
	for (i=1; i<n; i++) {
		dataExchanger->getGameName(i);
		printf("DEBUG - adding game: '%s'\n", dataExchanger->gameName);
		sprintf(temp_name, "%s - %s", dataExchanger->gameName, dataExchanger->centNumber);
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
	dataExchanger->loadGame(n);

	//printf("DEBUG - loadGamePicker::on_buttonBox_accepted - selectedItem: %d\n", ui->gameList->currentRow());
}
