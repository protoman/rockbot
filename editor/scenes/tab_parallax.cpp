#include "tab_parallax.h"
#include "ui_tab_parallax.h"

tab_parallax::tab_parallax(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tab_parallax)
{
    ui->setupUi(this);
}

tab_parallax::~tab_parallax()
{
    delete ui;
}
