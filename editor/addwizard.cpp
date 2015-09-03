#include <QtGui>

#include "addwizard.h"
#include <mediator.h>

extern Mediator *dataExchanger;

addWizard::addWizard(QWidget *parent) : QWizard(parent) {
    ui.setupUi(this);
    QObject::connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)));
    QObject::connect(ui.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_currentIndexChanged(int)));
    nextPageCount=0;
    ui.comboBox->addItem("Map");
    ui.comboBox->addItem("NPC");
    fillNPCGraphicsList();
	done = 0;
}

 void addWizard::pageChanged(int id) {
     printf("Current page: %d\n", id);
	 if (id == 3) {
		 if (done == 0) {
			 printf("Finishing, must add map or npc\n");
			 if (type == 0) {
				 printf("ADD MAP\n");
				 //dataExchanger->addMap();
			 }
			 done = 1;
			 emit finishedWizard();
			 printf("DEBUG - emit signal finishedWizard\n");
		 }
	 }
 }


int addWizard::nextId() const {
    if (this->currentId() == 0) {
        return nextPageCount;
    } else if (this->currentId() == 1) {
        return 3;
    } else if (this->currentId() == 2) {
        return 3;
    } else if (this->currentId() == 3) {
        return -1;
    }
	return -1;
}

void addWizard::on_comboBox_currentIndexChanged(int value)
{
	type = value;
    printf("DEBUG.addWizard::on_comboBox_currentIndexChanged - value: %d\n", value);
    if (value == 0) {
        printf("MAP\n");
        nextPageCount=2;
    } else {
        printf("NOT-MAP\n");
        nextPageCount=1;
    }
}

void addWizard::fillNPCGraphicsList() {
    printf("DEBUG - fillNPCGraphicsList - START\n");
    QDir dir = QDir("data/images/sprites/enemies/");
    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Size | QDir::Reversed);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
		//printf("addWizard::fillNPCGraphicsList - fileName: %s\n", qPrintable(fileInfo.fileName()));
		if (i == 0) {
			sprintf(dataExchanger->addNpcFilename, "data/images/sprites/enemies/%s", qPrintable(fileInfo.fileName()));
			printf(">> SET > dataExchanger::fillNPCGraphicsList->addNpcFilename: '%s'\n", dataExchanger->addNpcFilename);
		}
		//dataExchanger->NpcPreviewFile = new QString(fileInfo.fileName());
		//sprintf(dataExchanger->addNpcFilename, "%s", qPrintable(fileInfo.fileName()));
        ui.npcListCombobox->addItem(fileInfo.fileName());
    }
	//printf(">> dataExchanger::fillNPCGraphicsList->addNpcFilename: '%s'\n", dataExchanger->addNpcFilename);
	ui.npcPreviewAreaWidget->repaint();
}

void addWizard::on_npcGraphicSizeSpin_valueChanged(int value)
{
	dataExchanger->npcGraphicSize_w = value;
    ui.npcPreviewAreaWidget->repaint();
}

void addWizard::on_npcListCombobox_currentIndexChanged(QString item)
{
	//sprintf(dataExchanger->NpcPreviewFile, "data/images/sprites/enemies/%s", qPrintable(item));
	//dataExchanger->NpcPreviewFile = new QString("data/images/sprites/enemies/");
	//(*dataExchanger->NpcPreviewFile).append(item);
	sprintf(dataExchanger->addNpcFilename, "data/images/sprites/enemies/%s", qPrintable(item));
	//printf("DEBUG.addWizard::on_npcListCombobox_currentIndexChanged - fileName: %s\n", qPrintable(*dataExchanger->NpcPreviewFile));
	ui.npcPreviewAreaWidget->repaint();
}


