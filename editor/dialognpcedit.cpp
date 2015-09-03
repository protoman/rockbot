#include <stdio.h>
#include <QDir>
#include "dialognpcedit.h"
#include "ui_dialognpcedit.h"
#include "mediator.h"
#include "defines.h"
#include "../file/format.h".h"

extern std::vector<std::string> AI_ACTION_NAMES;

extern char EDITOR_FILEPATH[512];
extern std::string FILEPATH;
extern CURRENT_FILE_FORMAT::file_game game_data;

extern Mediator *dataExchanger;

DialogNPCEdit::DialogNPCEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNPCEdit)
{
	std::cout << "DialogNPCEdit::DialogNPCEdit::START" << std::endl;

    ui->setupUi(this);

    for (int i=0; i<AI_ACTION_NAMES.size(); i++) {
        std::string temp = AI_ACTION_NAMES[i];
        QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(temp.c_str());
        ui->comboBoxIA->addItem(temp_str);
	}
	ui->comboBoxIA->update();

	fillNPCGraphicsList();
	//printf("%%%% DialogNPCEdit::DialogNPCEdit - dataExchanger->selectedNPC: %d, editMode: %d\n", dataExchanger->selectedNPC, dataExchanger->editModeNPC);
    if (dataExchanger->editModeNPC == 1) {
		loadNPCData(dataExchanger->selectedNPC);
		std::cout << "DialogObjectEdit::DialogObjectEdit - editing npc, id: " << dataExchanger->selectedNPC << std::endl;
	} else {
		std::cout << "DialogObjectEdit::DialogObjectEdit - adding new npc, id: " << dataExchanger->selectedNPC << std::endl;
	}


    QObject::connect(ui->npcPreviewAreaWidget, SIGNAL(clickedIn()), this, SLOT(setNPCFrame()));
	std::cout << "DialogNPCEdit::DialogNPCEdit::END" << std::endl;
}




void DialogNPCEdit::loadNPCData(int npc_n) {
	std::cout << "DialogNPCEdit::loadNPCData::START" << std::endl;
	ui->NpcName->setText(game_data.game_npcs[npc_n].name);
	ui->NpcHP->setValue(game_data.game_npcs[npc_n].hp.total);
	ui->npcGraphicSizeSpin_w->setValue(game_data.game_npcs[npc_n].frame_size.width);
	ui->npcGraphicSizeSpin_h->setValue(game_data.game_npcs[npc_n].frame_size.height);
	dataExchanger->npcGraphicSize_w = game_data.game_npcs[npc_n].frame_size.width;
	dataExchanger->npcGraphicSize_h = game_data.game_npcs[npc_n].frame_size.height;
	for (int i=0; i<ui->npcListCombobox->count(); i++) {
		if (ui->npcListCombobox->itemText(i) == QString(game_data.game_npcs[npc_n].graphic_filename)) {
			ui->npcListCombobox->setCurrentIndex(i);
			break;
		}
	}
    if (game_data.game_npcs[npc_n].is_ghost) {
		ui->checkBoxCanShoot->setChecked(true);
	}
	/// @TODO
	/*
	if (temp_npc.is_boss == 1) {
		ui->isBossCheckbox->setChecked(true);
	}
	*/
	ui->comboBoxShieldType->setCurrentIndex(game_data.game_npcs[npc_n].shield_type);

	ui->comboBoxIA->setCurrentIndex(game_data.game_npcs[npc_n].IA_type);

	ui->speed_spin->setValue(game_data.game_npcs[npc_n].speed);
	ui->range_spin->setValue(game_data.game_npcs[npc_n].walk_range);
	std::cout << "DialogNPCEdit::loadNPCData::END" << std::endl;
}



void DialogNPCEdit::saveNPCData(int npc_n) {
	if (game_data.game_npcs[npc_n].id == -1) {
		game_data.game_npcs[npc_n].id = npc_n;
	}
	sprintf(game_data.game_npcs[npc_n].name, "%s", ui->NpcName->text().toStdString().c_str());
	game_data.game_npcs[npc_n].hp.total = ui->NpcHP->value();
	game_data.game_npcs[npc_n].frame_size.width = ui->npcGraphicSizeSpin_w->value();
	game_data.game_npcs[npc_n].frame_size.height = ui->npcGraphicSizeSpin_h->value();
	sprintf(game_data.game_npcs[npc_n].graphic_filename, ui->npcListCombobox->currentText().toStdString().c_str());

	if (ui->checkBoxCanShoot->isChecked()) {
        game_data.game_npcs[npc_n].is_ghost=1;
	} else {
        game_data.game_npcs[npc_n].is_ghost=0;
	}

	game_data.game_npcs[npc_n].shield_type = ui->comboBoxShieldType->currentIndex();

	game_data.game_npcs[npc_n].IA_type = ui->comboBoxIA->currentIndex();
	game_data.game_npcs[npc_n].speed = ui->speed_spin->value();
	game_data.game_npcs[npc_n].walk_range = ui->range_spin->value();



}



DialogNPCEdit::~DialogNPCEdit()
{
    delete ui;
}

void DialogNPCEdit::changeEvent(QEvent *e)
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


void DialogNPCEdit::fillNPCGraphicsList() {
	printf("DEBUG - fillNPCGraphicsList - START\n");
    //qDebug(QString("dir: "+QString(EDITOR_FILEPATH)+"/data/images/sprites/enemies/"));
    QDir dir = QDir(QString(EDITOR_FILEPATH)+"/data/images/sprites/enemies/");
    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Size | QDir::Reversed);
    QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		//printf("DialogNPCEdit::fillNPCGraphicsList - fileName: %s\n", qPrintable(fileInfo.fileName()));
		if (i == 0) {
			sprintf(dataExchanger->addNpcFilename, "%s/data/images/sprites/enemies/%s", EDITOR_FILEPATH, qPrintable(fileInfo.fileName()));
			printf(">> SET > dataExchanger::fillNPCGraphicsList->addNpcFilename: '%s'\n", dataExchanger->addNpcFilename);
		}
		//dataExchanger->NpcPreviewFile = new QString(fileInfo.fileName());
		//sprintf(dataExchanger->addNpcFilename, "%s", qPrintable(fileInfo.fileName()));
		ui->npcListCombobox->addItem(fileInfo.fileName());
	}
	//printf(">> dataExchanger::fillNPCGraphicsList->addNpcFilename: '%s'\n", dataExchanger->addNpcFilename);
	ui->npcPreviewAreaWidget->repaint();
}




void DialogNPCEdit::on_npcGraphicSizeSpin_w_valueChanged(int value)
{
	dataExchanger->npcGraphicSize_w = value;
	ui->npcPreviewAreaWidget->repaint();
}

void DialogNPCEdit::on_npcGraphicSizeSpin_h_valueChanged(int value)
{
	dataExchanger->npcGraphicSize_h = value;
	ui->npcPreviewAreaWidget->repaint();
}

void DialogNPCEdit::on_npcListCombobox_currentIndexChanged(QString item)
{
	sprintf(dataExchanger->addNpcFilename, "%s/data/images/sprites/enemies/%s", EDITOR_FILEPATH, qPrintable(item));
	ui->npcPreviewAreaWidget->repaint();
}

void DialogNPCEdit::on_buttonBox_accepted()
{
	printf(">> DialogNPCEdit::on_buttonBox_accepted\n");
	saveNPCData(dataExchanger->selectedNPC);
	emit finishedNPCEditor();
}

void DialogNPCEdit::setNPCFrame() {
	printf(">> DialogNPCEdit::setNPCFrame - CALLED\n");
    // @204
    //ui->frameTypeCombo->setCurrentIndex(dataExchanger->frameset[dataExchanger->getPalleteX()]+1);
    //ui->spinBox->setValue(dataExchanger->frameset_time[dataExchanger->getPalleteX()]);
}

void DialogNPCEdit::on_frameTypeCombo_currentIndexChanged(int index)
{
    // @204
    //dataExchanger->frameset[dataExchanger->getPalleteX()] = index-1;
}

void DialogNPCEdit::on_spinBox_valueChanged(int value)
{
    // @204
    //dataExchanger->frameset_time[dataExchanger->getPalleteX()] = value;
}
