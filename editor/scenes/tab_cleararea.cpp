#include "tab_cleararea.h"
#include "ui_tab_cleararea.h"
#include "mediator.h"

#include <QColorDialog>

TabClearArea::TabClearArea(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabClearArea)
{
    ui->setupUi(this);

    fill_data();
}

TabClearArea::~TabClearArea()
{
    delete ui;
}

void TabClearArea::save_data()
{
    fio.save_scenes_clear_area(ScenesMediator::get_instance()->cleararea_list);
}

void TabClearArea::reload()
{
    fill_data();
}

void TabClearArea::change_fields_enabled(bool state)
{
    ui->x_spinBox->setEnabled(state);
    ui->y_spinBox->setEnabled(state);
    ui->w_spinBox->setEnabled(state);
    ui->h_spinBox->setEnabled(state);
    ui->color_pushButton->setEnabled(state);
}

void TabClearArea::fill_data()
{
    data_loading = true;

    ScenesMediator::get_instance()->cleararea_list = fio.load_scenes_clear_area();
    int list_size = ScenesMediator::get_instance()->cleararea_list.size();

    if (list_size == 0) {
        change_fields_enabled(false);
    } else {
        change_fields_enabled(true);
        for (int i=0; i<list_size; i++) {
            ui->select_comboBox->addItem(QString(ScenesMediator::get_instance()->cleararea_list.at(i).name));
        }
        set_fields(0);

    }
    data_loading = false;
    ui->select_comboBox->setCurrentIndex(0);
}

void TabClearArea::set_fields(int index)
{
    ui->name_lineEdit->setText(ScenesMediator::get_instance()->cleararea_list.at(index).name);
    ui->x_spinBox->setValue(ScenesMediator::get_instance()->cleararea_list.at(index).x);
    ui->y_spinBox->setValue(ScenesMediator::get_instance()->cleararea_list.at(index).y);
    ui->w_spinBox->setValue(ScenesMediator::get_instance()->cleararea_list.at(index).w);
    ui->h_spinBox->setValue(ScenesMediator::get_instance()->cleararea_list.at(index).h);
    reload_button_color(index);
}

void TabClearArea::reload_button_color(int index)
{
    QString color_css = QString("background-color: rgb(") + QString::number(ScenesMediator::get_instance()->cleararea_list.at(index).r) + QString(", ") + QString::number(ScenesMediator::get_instance()->cleararea_list.at(index).g) + QString(", ") + QString::number(ScenesMediator::get_instance()->cleararea_list.at(index).b) + QString(")");
    ui->color_pushButton->setStyleSheet(color_css);
}

void TabClearArea::on_add_pushButton_clicked()
{
    CURRENT_FILE_FORMAT::file_scene_clear_area new_cleararea;
    sprintf(new_cleararea.name, "%s%d", "Clear Area #", ScenesMediator::get_instance()->cleararea_list.size()+1);
    ScenesMediator::get_instance()->cleararea_list.push_back(new_cleararea);
    ui->select_comboBox->addItem(QString(new_cleararea.name));
    if (ScenesMediator::get_instance()->cleararea_list.size() == 1) {
        change_fields_enabled(true);
    }
    ui->select_comboBox->setCurrentIndex(ScenesMediator::get_instance()->cleararea_list.size()-1);
}

void TabClearArea::on_select_comboBox_currentIndexChanged(int index)
{
    if (data_loading == true) { return; }
    set_fields(index);
}

void TabClearArea::on_color_pushButton_clicked()
{

    QColorDialog *colorDialog = new QColorDialog(this);
    QObject::connect(colorDialog, SIGNAL(colorSelected(QColor)), this, SLOT(pick_color_slot(QColor)));
    colorDialog->show();
}

void TabClearArea::pick_color_slot(QColor color)
{
    if (data_loading == true) { return; }
    if (Mediator::get_instance()->picked_color_n != -1) {
        ScenesMediator::get_instance()->cleararea_list.at(ui->select_comboBox->currentIndex()).r = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].r;
        ScenesMediator::get_instance()->cleararea_list.at(ui->select_comboBox->currentIndex()).g = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].g;
        ScenesMediator::get_instance()->cleararea_list.at(ui->select_comboBox->currentIndex()).b = Mediator::get_instance()->colormap[Mediator::get_instance()->picked_color_n].b;
        reload_button_color(ui->select_comboBox->currentIndex());
    }
}

void TabClearArea::on_name_lineEdit_textChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    sprintf(ScenesMediator::get_instance()->cleararea_list.at(ui->select_comboBox->currentIndex()).name, "%s", arg1.toStdString().c_str());
}

void TabClearArea::on_x_spinBox_valueChanged(int arg1)
{
    if (data_loading == true) { return; }
    ScenesMediator::get_instance()->cleararea_list.at(ui->select_comboBox->currentIndex()).x = arg1;
}

void TabClearArea::on_y_spinBox_valueChanged(int arg1)
{
    if (data_loading == true) { return; }
    ScenesMediator::get_instance()->cleararea_list.at(ui->select_comboBox->currentIndex()).y = arg1;
}

void TabClearArea::on_w_spinBox_valueChanged(int arg1)
{
    if (data_loading == true) { return; }
    ScenesMediator::get_instance()->cleararea_list.at(ui->select_comboBox->currentIndex()).w = arg1;
}

void TabClearArea::on_h_spinBox_valueChanged(int arg1)
{
    if (data_loading == true) { return; }
    ScenesMediator::get_instance()->cleararea_list.at(ui->select_comboBox->currentIndex()).h = arg1;
}
