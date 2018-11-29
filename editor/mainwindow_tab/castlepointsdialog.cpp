#include "castlepointsdialog.h"
#include "ui_castlepointsdialog.h"

CastlePointsDialog::CastlePointsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CastlePointsDialog)
{
    ui->setupUi(this);
}

CastlePointsDialog::~CastlePointsDialog()
{
    delete ui;
}
