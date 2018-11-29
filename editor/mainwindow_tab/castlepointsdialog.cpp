#include "castlepointsdialog.h"
#include "ui_castlepointsdialog.h"

CastlePointsDialog::CastlePointsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CastlePointsDialog)
{
    data_loading = true;
    ui->setupUi(this);
    common::fill_numbered_combo(ui->point_comboBox, 0, 5);
    load_data();
    data_loading = false;
}

CastlePointsDialog::~CastlePointsDialog()
{
    delete ui;
}

void CastlePointsDialog::load_data()
{
    data_loading = true;
    currentPoint = 0;
    ui->point_comboBox->setCurrentIndex(currentPoint);
    std::cout << "Mediator::get_instance()->points_castle1.points[currentPoint].x[" << Mediator::get_instance()->points_castle1.points[currentPoint].x << "]" << std::endl;
    ui->x_spinBox->setValue(Mediator::get_instance()->points_castle1.points[currentPoint].x);
    ui->y_spinBox->setValue(Mediator::get_instance()->points_castle1.points[currentPoint].y);
    data_loading = false;

}

void CastlePointsDialog::on_point_comboBox_currentIndexChanged(int index)
{
    if (data_loading) {
        return;
    }
    data_loading = true;
    currentPoint = index;
    ui->x_spinBox->setValue(Mediator::get_instance()->points_castle1.points[currentPoint].x);
    ui->y_spinBox->setValue(Mediator::get_instance()->points_castle1.points[currentPoint].y);
    ui->drawArea_widget->set_current_point(currentPoint);
    ui->drawArea_widget->repaint();
    data_loading = false;
}

void CastlePointsDialog::on_x_spinBox_valueChanged(int arg1)
{
    if (data_loading) {
        return;
    }
    Mediator::get_instance()->points_castle1.points[currentPoint].x = arg1;
    ui->drawArea_widget->repaint();
}

void CastlePointsDialog::on_y_spinBox_valueChanged(int arg1)
{
    if (data_loading) {
        return;
    }
    Mediator::get_instance()->points_castle1.points[currentPoint].y = arg1;
    ui->drawArea_widget->repaint();
}

