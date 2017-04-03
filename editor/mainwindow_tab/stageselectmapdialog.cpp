#include "stageselectmapdialog.h"
#include "ui_stageselectmapdialog.h"

StageSelectMapDialog::StageSelectMapDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StageSelectMapDialog)
{
    ui->setupUi(this);
}

StageSelectMapDialog::~StageSelectMapDialog()
{
    delete ui;
}
