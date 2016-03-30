#include <stdio.h>
#include <QDir>
#include "dialognpcedit.h"
#include "ui_dialognpcedit.h"
#include "mediator.h"
#include "defines.h"
#include "../file/format.h".h"

extern std::vector<std::string> AI_ACTION_NAMES;
extern std::string GAMEPATH;

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
    //printf("%%%% DialogNPCEdit::DialogNPCEdit - Mediator::get_instance()->selectedNPC: %d, editMode: %d\n", Mediator::get_instance()->selectedNPC, Mediator::get_instance()->editModeNPC);
    if (Mediator::get_instance()->editModeNPC == 1) {
        loadNPCData(Mediator::get_instance()->selectedNPC);
        std::cout << "DialogObjectEdit::DialogObjectEdit - editing npc, id: " << Mediator::get_instance()->selectedNPC << std::endl;
	} else {
        std::cout << "DialogObjectEdit::DialogObjectEdit - adding new npc, id: " << Mediator::get_instance()->selectedNPC << std::endl;
	}


    QObject::connect(ui->npcPreviewAreaWidget, SIGNAL(clickedIn()), this, SLOT(setNPCFrame()));
	std::cout << "DialogNPCEdit::DialogNPCEdit::END" << std::endl;
}



void DialogNPCEdit::loadNPCData(int npc_n) {
	std::cout << "DialogNPCEdit::loadNPCData::START" << std::endl;
    ui->NpcName->setText(Mediator::get_instance()->enemy_list.at(npc_n).name);
    ui->NpcHP->setValue(Mediator::get_instance()->enemy_list.at(npc_n).hp.total);
    ui->npcGraphicSizeSpin_w->setValue(Mediator::get_instance()->enemy_list.at(npc_n).frame_size.width);
    ui->npcGraphicSizeSpin_h->setValue(Mediator::get_instance()->enemy_list.at(npc_n).frame_size.height);
    Mediator::get_instance()->npcGraphicSize_w = Mediator::get_instance()->enemy_list.at(npc_n).frame_size.width;
    Mediator::get_instance()->npcGraphicSize_h = Mediator::get_instance()->enemy_list.at(npc_n).frame_size.height;
	for (int i=0; i<ui->npcListCombobox->count(); i++) {
        if (ui->npcListCombobox->itemText(i) == QString(Mediator::get_instance()->enemy_list.at(npc_n).graphic_filename)) {
			ui->npcListCombobox->setCurrentIndex(i);
			break;
		}
	}
    if (Mediator::get_instance()->enemy_list.at(npc_n).is_ghost) {
		ui->checkBoxCanShoot->setChecked(true);
	}
	/// @TODO
	/*
	if (temp_npc.is_boss == 1) {
		ui->isBossCheckbox->setChecked(true);
	}
	*/
    ui->comboBoxShieldType->setCurrentIndex(Mediator::get_instance()->enemy_list.at(npc_n).shield_type);

    ui->speed_spin->setValue(Mediator::get_instance()->enemy_list.at(npc_n).speed);
    ui->range_spin->setValue(Mediator::get_instance()->enemy_list.at(npc_n).walk_range);
	std::cout << "DialogNPCEdit::loadNPCData::END" << std::endl;
}



void DialogNPCEdit::saveNPCData(int npc_n) {
    if (Mediator::get_instance()->enemy_list.at(npc_n).id == -1) {
        Mediator::get_instance()->enemy_list.at(npc_n).id = npc_n;
	}
    sprintf(Mediator::get_instance()->enemy_list.at(npc_n).name, "%s", ui->NpcName->text().toStdString().c_str());
    Mediator::get_instance()->enemy_list.at(npc_n).hp.total = ui->NpcHP->value();
    Mediator::get_instance()->enemy_list.at(npc_n).frame_size.width = ui->npcGraphicSizeSpin_w->value();
    Mediator::get_instance()->enemy_list.at(npc_n).frame_size.height = ui->npcGraphicSizeSpin_h->value();
    sprintf(Mediator::get_instance()->enemy_list.at(npc_n).graphic_filename, ui->npcListCombobox->currentText().toStdString().c_str());

	if (ui->checkBoxCanShoot->isChecked()) {
        Mediator::get_instance()->enemy_list.at(npc_n).is_ghost=1;
	} else {
        Mediator::get_instance()->enemy_list.at(npc_n).is_ghost=0;
	}

    Mediator::get_instance()->enemy_list.at(npc_n).shield_type = ui->comboBoxShieldType->currentIndex();

    Mediator::get_instance()->enemy_list.at(npc_n).speed = ui->speed_spin->value();
    Mediator::get_instance()->enemy_list.at(npc_n).walk_range = ui->range_spin->value();



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
    QDir dir = QDir(QString(FILEPATH.c_str())+"/images/sprites/enemies/");
    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Size | QDir::Reversed);
    QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		//printf("DialogNPCEdit::fillNPCGraphicsList - fileName: %s\n", qPrintable(fileInfo.fileName()));
		if (i == 0) {
            sprintf(Mediator::get_instance()->addNpcFilename, "%s/images/sprites/enemies/%s", FILEPATH.c_str(), qPrintable(fileInfo.fileName()));
            printf(">> SET > dataExchanger::fillNPCGraphicsList->addNpcFilename: '%s'\n", Mediator::get_instance()->addNpcFilename);
		}
        //Mediator::get_instance()->NpcPreviewFile = new QString(fileInfo.fileName());
        //sprintf(Mediator::get_instance()->addNpcFilename, "%s", qPrintable(fileInfo.fileName()));
		ui->npcListCombobox->addItem(fileInfo.fileName());
	}
    //printf(">> dataExchanger::fillNPCGraphicsList->addNpcFilename: '%s'\n", Mediator::get_instance()->addNpcFilename);
	ui->npcPreviewAreaWidget->repaint();
}




void DialogNPCEdit::on_npcGraphicSizeSpin_w_valueChanged(int value)
{
    Mediator::get_instance()->npcGraphicSize_w = value;
	ui->npcPreviewAreaWidget->repaint();
}

void DialogNPCEdit::on_npcGraphicSizeSpin_h_valueChanged(int value)
{
    Mediator::get_instance()->npcGraphicSize_h = value;
	ui->npcPreviewAreaWidget->repaint();
}

void DialogNPCEdit::on_npcListCombobox_currentIndexChanged(QString item)
{
    sprintf(Mediator::get_instance()->addNpcFilename, "%s/images/sprites/enemies/%s", GAMEPATH.c_str(), qPrintable(item));
	ui->npcPreviewAreaWidget->repaint();
}

void DialogNPCEdit::on_buttonBox_accepted()
{
	printf(">> DialogNPCEdit::on_buttonBox_accepted\n");
    saveNPCData(Mediator::get_instance()->selectedNPC);
	emit finishedNPCEditor();
}

void DialogNPCEdit::setNPCFrame() {
	printf(">> DialogNPCEdit::setNPCFrame - CALLED\n");
    // @204
    //ui->frameTypeCombo->setCurrentIndex(Mediator::get_instance()->frameset[Mediator::get_instance()->getPalleteX()]+1);
    //ui->spinBox->setValue(Mediator::get_instance()->frameset_time[Mediator::get_instance()->getPalleteX()]);
}

void DialogNPCEdit::on_frameTypeCombo_currentIndexChanged(int index)
{
    // @204
    //Mediator::get_instance()->frameset[Mediator::get_instance()->getPalleteX()] = index-1;
}

void DialogNPCEdit::on_spinBox_valueChanged(int value)
{
    // @204
    //Mediator::get_instance()->frameset_time[Mediator::get_instance()->getPalleteX()] = value;
}
