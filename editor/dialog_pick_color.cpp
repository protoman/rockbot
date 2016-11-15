#include "dialog_pick_color.h"
#include "ui_dialog_pick_color.h"
#include <QPushButton>

#include "mediator.h"


dialog_pick_color::dialog_pick_color(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialog_pick_color)
{


	QPushButton* buttons[67];

    ui->setupUi(this);
	int column = 0;
	int row = 0;

	signalMapper = new QSignalMapper(this);

	// no-color button
	QPushButton *button = new QPushButton("");
	button->setMaximumWidth(32);
	button->setMaximumHeight(32);
	button->setStyleSheet(QString("border-style: outset; border-width: 1px; border-color: black;"));
	connect(button, SIGNAL(clicked()), signalMapper, SLOT(map()));
	signalMapper->setMapping(button, -1);
	ui->gridLayout->addWidget(button, row, column);
	column++;

	for (int i=0; i<67; i++) {
		QPushButton *button = new QPushButton("");
		button->setMaximumWidth(32);
		button->setMaximumHeight(32);
        button->setStyleSheet(QString("background-color: rgb(") + QString::number(Mediator::get_instance()->colormap[i].r) + QString(", ") +  QString::number(Mediator::get_instance()->colormap[i].g) + QString(", ") +  QString::number(Mediator::get_instance()->colormap[i].b) + QString("); border-style: outset; border-width: 1px; border-color: black;"));
		connect(button, SIGNAL(clicked()), signalMapper, SLOT(map()));
		signalMapper->setMapping(button, i);
		ui->gridLayout->addWidget(button, row, column);
		column++;
		if (column >= 16) {
			row++;
			column = 0;
		}
	}
	connect(signalMapper, SIGNAL(mapped(const int &)), this, SLOT(clicked(const int &)));
}

dialog_pick_color::~dialog_pick_color()
{
    delete ui;
}

void dialog_pick_color::clicked(const int &color_n)
{
	std::cout << "dialog_pick_color::clicked - color_n: " << color_n << std::endl;
    Mediator::get_instance()->picked_color_n = color_n;
}

void dialog_pick_color::on_buttonBox_accepted()
{
	emit accepted();
}
