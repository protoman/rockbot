#include <QtGui>

#include "addwizard.h"
#include <mediator.h>

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
	 if (id == 3) {
		 if (done == 0) {
			 done = 1;
			 emit finishedWizard();
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
    if (value == 0) {
        nextPageCount=2;
    } else {
        nextPageCount=1;
    }
}

void addWizard::fillNPCGraphicsList() {
    std::string file_dir = FILEPATH+std::string("/images/sprites/enemies/");
    QDir dir = QDir(file_dir.c_str());
    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Size | QDir::Reversed);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
		//printf("addWizard::fillNPCGraphicsList - fileName: %s\n", qPrintable(fileInfo.fileName()));
		if (i == 0) {
            sprintf(Mediator::get_instance()->addNpcFilename, "images/sprites/enemies/%s", qPrintable(fileInfo.fileName()));
		}
        //Mediator::get_instance()->NpcPreviewFile = new QString(fileInfo.fileName());
        //sprintf(Mediator::get_instance()->addNpcFilename, "%s", qPrintable(fileInfo.fileName()));
        ui.npcListCombobox->addItem(fileInfo.fileName());
    }
    //printf(">> dataExchanger::fillNPCGraphicsList->addNpcFilename: '%s'\n", Mediator::get_instance()->addNpcFilename);
	ui.npcPreviewAreaWidget->repaint();
}

void addWizard::on_npcGraphicSizeSpin_valueChanged(int value)
{
    Mediator::get_instance()->npcGraphicSize_w = value;
    ui.npcPreviewAreaWidget->repaint();
}

void addWizard::on_npcListCombobox_currentIndexChanged(QString item)
{
    //sprintf(Mediator::get_instance()->NpcPreviewFile, "images/sprites/enemies/%s", qPrintable(item));
    //Mediator::get_instance()->NpcPreviewFile = new QString("images/sprites/enemies/");
    //(*Mediator::get_instance()->NpcPreviewFile).append(item);
    sprintf(Mediator::get_instance()->addNpcFilename, "images/sprites/enemies/%s", qPrintable(item));
    //printf("DEBUG.addWizard::on_npcListCombobox_currentIndexChanged - fileName: %s\n", qPrintable(*Mediator::get_instance()->NpcPreviewFile));
	ui.npcPreviewAreaWidget->repaint();
}


