#include "tab_viewpoint.h"
#include "ui_tab_viewpoint.h"

tab_viewpoint::tab_viewpoint(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tab_viewpoint)
{
    ui->setupUi(this);
}

tab_viewpoint::~tab_viewpoint()
{
    delete ui;
}
