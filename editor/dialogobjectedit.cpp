#include "dialogobjectedit.h"
#include "ui_dialogobjectedit.h"
#include "../file/format.h"
#include <QDir>
#include <QMessageBox>

extern char EDITOR_FILEPATH[512];
extern CURRENT_FILE_FORMAT::file_game game_data;

DialogObjectEdit::DialogObjectEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogObjectEdit)
{
	ui->setupUi(this);
	fillObjectGraphicsList();
	if (dataExchanger->editModeNPC == 1) {
		std::cout << "DialogObjectEdit::DialogObjectEdit - editing existing Obj, id: " << dataExchanger->selectedNPC << std::endl;
		loadObjectData(dataExchanger->selectedNPC);
	} else {
		std::cout << "DialogObjectEdit::DialogObjectEdit - adding new Obj, id: " << dataExchanger->selectedNPC << std::endl;
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
	//ui->frameTypeCombo->setCurrentIndex(dataExchanger->frameset[dataExchanger->getPalleteX()]+1);
	//ui->spinBox->setValue(dataExchanger->frameset_time[dataExchanger->getPalleteX()]);
}

void DialogObjectEdit::on_buttonBox_accepted()
{
	if (ui->NpcName->text().contains(".", Qt::CaseInsensitive)) {
		QMessageBox::warning(this, "Error", "Object name cannot contain dot character.");
		return;
	}
	printf(">> DialogObjectEdit::on_buttonBox_accepted\n");
	saveObjectData(dataExchanger->selectedNPC);
	emit finishedObjectEditor();
	close();
}

void DialogObjectEdit::fillObjectGraphicsList() {
	QString dirPath(QString(EDITOR_FILEPATH)+"/data/images/sprites/objects/");
    QDir dir = QDir(dirPath);
    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Size | QDir::Reversed);
    QFileInfoList list = dir.entryInfoList();
	//qDebug (QString (">> fillObjectGraphicsList - dir: " + dirPath + ", list.size: " + QString::number(list.size ())).toAscii());
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileInfo = list.at(i);
		if (i == 0) {
			sprintf(dataExchanger->addNpcFilename, "%s/data/images/sprites/objects/%s", EDITOR_FILEPATH, qPrintable(fileInfo.fileName()));
		}
		ui->objectListCombobox->addItem(fileInfo.fileName());
	}
	ui->npcPreviewAreaWidget->repaint();
}

void DialogObjectEdit::on_npcGraphicSizeSpin_w_valueChanged(int value)
{
	dataExchanger->npcGraphicSize_w = value;
	ui->npcPreviewAreaWidget->repaint();
}

void DialogObjectEdit::on_npcGraphicSizeSpin_h_valueChanged(int value)
{
	dataExchanger->npcGraphicSize_h = value;
	ui->npcPreviewAreaWidget->repaint();
}

void DialogObjectEdit::on_objectListCombobox_currentIndexChanged(QString item)
{
	sprintf(dataExchanger->addNpcFilename, "%s/data/images/sprites/objects/%s", EDITOR_FILEPATH, qPrintable(item));
	ui->npcPreviewAreaWidget->repaint();
}



void DialogObjectEdit::loadObjectData(int object_n) {

	ui->NpcName->setText(game_data.objects[object_n].name);
	ui->npcGraphicSizeSpin_w->setValue(game_data.objects[object_n].size.width);
	ui->npcGraphicSizeSpin_h->setValue(game_data.objects[object_n].size.height);
	ui->comboBoxType->setCurrentIndex(game_data.objects[object_n].type);

	ui->spinBox_timer->setValue(game_data.objects[object_n].timer);
	ui->spinBox_speed->setValue(game_data.objects[object_n].speed);
	ui->spinBox_limit->setValue(game_data.objects[object_n].limit);

	dataExchanger->npcGraphicSize_w = game_data.objects[object_n].size.width;
	dataExchanger->npcGraphicSize_h = game_data.objects[object_n].size.height;
	for (int i=0; i<ui->objectListCombobox->count(); i++) {
		if (ui->objectListCombobox->itemText(i) == QString(game_data.objects[object_n].graphic_filename)) {
			ui->objectListCombobox->setCurrentIndex(i);
			break;
		}
	}
}



void DialogObjectEdit::saveObjectData(int object_n) {
	strcpy (game_data.objects[object_n].name, ui->NpcName->text().toLatin1());
	game_data.objects[object_n].type = ui->comboBoxType->currentIndex();
	game_data.objects[object_n].size.width = ui->npcGraphicSizeSpin_w->value();
	game_data.objects[object_n].size.height = ui->npcGraphicSizeSpin_h->value();
	strcpy (game_data.objects[object_n].graphic_filename, ui->objectListCombobox->currentText().toLatin1());
	game_data.objects[object_n].timer = ui->spinBox_timer->value();
	game_data.objects[object_n].speed = ui->spinBox_speed->value();
	game_data.objects[object_n].limit = ui->spinBox_limit->value();
}


void DialogObjectEdit::on_buttonBox_rejected()
{
	close();
}

void DialogObjectEdit::on_NpcName_textChanged(const QString &arg1)
{

}
