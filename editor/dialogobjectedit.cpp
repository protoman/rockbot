#include "dialogobjectedit.h"
#include "ui_dialogobjectedit.h"
#include "../file/format.h"
#include <QDir>
#include <QMessageBox>

DialogObjectEdit::DialogObjectEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogObjectEdit)
{
	ui->setupUi(this);
	fillObjectGraphicsList();
	if (Mediator::get_instance()->editModeNPC == 1) {
		std::cout << "DialogObjectEdit::DialogObjectEdit - editing existing Obj, id: " << Mediator::get_instance()->selectedNPC << std::endl;
		loadObjectData(Mediator::get_instance()->selectedNPC);
	} else {
		std::cout << "DialogObjectEdit::DialogObjectEdit - adding new Obj, id: " << Mediator::get_instance()->selectedNPC << std::endl;
	}
	QObject::connect(ui->npcPreviewAreaWidget, SIGNAL(clickedIn()), this, SLOT(setObjectFrame()));
}

DialogObjectEdit::~DialogObjectEdit()
{
    delete ui;
}

void DialogObjectEdit::changeEvent(QEvent *e)
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

void DialogObjectEdit::setObjectFrame() {
	//printf(">> DialogObjectEdit::setObjectFrame - CALLED\n");
	//ui->frameTypeCombo->setCurrentIndex(Mediator::get_instance()->frameset[Mediator::get_instance()->getPalleteX()]+1);
	//ui->spinBox->setValue(Mediator::get_instance()->frameset_time[Mediator::get_instance()->getPalleteX()]);
}

void DialogObjectEdit::on_buttonBox_accepted()
{
	if (ui->NpcName->text().contains(".", Qt::CaseInsensitive)) {
		QMessageBox::warning(this, "Error", "Object name cannot contain dot character.");
		return;
	}
	printf(">> DialogObjectEdit::on_buttonBox_accepted\n");
	saveObjectData(Mediator::get_instance()->selectedNPC);
	emit finishedObjectEditor();
	close();
}

void DialogObjectEdit::fillObjectGraphicsList() {
    QString dirPath(QString(FILEPATH.c_str())+"/images/sprites/objects/");
    QDir dir = QDir(dirPath);
    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Size | QDir::Reversed);
    QFileInfoList list = dir.entryInfoList();
	//qDebug (QString (">> fillObjectGraphicsList - dir: " + dirPath + ", list.size: " + QString::number(list.size ())).toAscii());
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		if (i == 0) {
            sprintf(Mediator::get_instance()->addNpcFilename, "%s/images/sprites/objects/%s", FILEPATH.c_str(), qPrintable(fileInfo.fileName()));
		}
		ui->objectListCombobox->addItem(fileInfo.fileName());
	}
	ui->npcPreviewAreaWidget->repaint();
}

void DialogObjectEdit::on_npcGraphicSizeSpin_w_valueChanged(int value)
{
	Mediator::get_instance()->npcGraphicSize_w = value;
	ui->npcPreviewAreaWidget->repaint();
}

void DialogObjectEdit::on_npcGraphicSizeSpin_h_valueChanged(int value)
{
	Mediator::get_instance()->npcGraphicSize_h = value;
	ui->npcPreviewAreaWidget->repaint();
}

void DialogObjectEdit::on_objectListCombobox_currentIndexChanged(QString item)
{
    sprintf(Mediator::get_instance()->addNpcFilename, "%s/images/sprites/objects/%s", FILEPATH.c_str(), qPrintable(item));
	ui->npcPreviewAreaWidget->repaint();
}



void DialogObjectEdit::loadObjectData(int object_n) {

    ui->NpcName->setText(Mediator::get_instance()->object_list.at(object_n).name);
    ui->npcGraphicSizeSpin_w->setValue(Mediator::get_instance()->object_list.at(object_n).size.width);
    ui->npcGraphicSizeSpin_h->setValue(Mediator::get_instance()->object_list.at(object_n).size.height);
    ui->comboBoxType->setCurrentIndex(Mediator::get_instance()->object_list.at(object_n).type);

    ui->spinBox_timer->setValue(Mediator::get_instance()->object_list.at(object_n).timer);
    ui->spinBox_speed->setValue(Mediator::get_instance()->object_list.at(object_n).speed);
    ui->spinBox_limit->setValue(Mediator::get_instance()->object_list.at(object_n).limit);

    Mediator::get_instance()->npcGraphicSize_w = Mediator::get_instance()->object_list.at(object_n).size.width;
    Mediator::get_instance()->npcGraphicSize_h = Mediator::get_instance()->object_list.at(object_n).size.height;
	for (int i=0; i<ui->objectListCombobox->count(); i++) {
        if (ui->objectListCombobox->itemText(i) == QString(Mediator::get_instance()->object_list.at(object_n).graphic_filename)) {
			ui->objectListCombobox->setCurrentIndex(i);
			break;
		}
	}
}



void DialogObjectEdit::saveObjectData(int object_n) {
    strcpy (Mediator::get_instance()->object_list.at(object_n).name, ui->NpcName->text().toLatin1());
    Mediator::get_instance()->object_list.at(object_n).type = ui->comboBoxType->currentIndex();
    Mediator::get_instance()->object_list.at(object_n).size.width = ui->npcGraphicSizeSpin_w->value();
    Mediator::get_instance()->object_list.at(object_n).size.height = ui->npcGraphicSizeSpin_h->value();
    strcpy (Mediator::get_instance()->object_list.at(object_n).graphic_filename, ui->objectListCombobox->currentText().toLatin1());
    Mediator::get_instance()->object_list.at(object_n).timer = ui->spinBox_timer->value();
    Mediator::get_instance()->object_list.at(object_n).speed = ui->spinBox_speed->value();
    Mediator::get_instance()->object_list.at(object_n).limit = ui->spinBox_limit->value();
}


void DialogObjectEdit::on_buttonBox_rejected()
{
	close();
}

void DialogObjectEdit::on_NpcName_textChanged(const QString &arg1)
{

}
