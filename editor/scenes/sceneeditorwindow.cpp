#include "sceneeditorwindow.h"
#include "ui_sceneeditorwindow.h"

SceneEditorWindow::SceneEditorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SceneEditorWindow)
{
    ui->setupUi(this);

    text_tab = new TabText();
    ui->tabTextScrollArea->setWidget(text_tab);

    image_tab = new tab_image();
    ui->tabImage_scrollArea->setWidget(image_tab);
    connect(ui->widget, SIGNAL(play_state_changed(bool)), this, SLOT(on_play_state_changed(bool)));

    viewpoint_tab = new tab_viewpoint();
    ui->tabViewPoint_scrollArea->setWidget(viewpoint_tab);
}

SceneEditorWindow::~SceneEditorWindow()
{
    delete ui;
}

void SceneEditorWindow::on_actionSave_triggered()
{
    image_tab->save_data();
    viewpoint_tab->save_data();
}

void SceneEditorWindow::on_pushButton_clicked()
{
    ui->widget->change_play_state();
}

void SceneEditorWindow::on_play_state_changed(bool state)
{
    if (state == true) {
        ui->pushButton->setText("STOP");
    } else {
        ui->pushButton->setText("PLAY");
    }
}
