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

    viewpoint_tab = new tab_viewpoint();
    ui->tabViewPoint_scrollArea->setWidget(viewpoint_tab);

    scenes_tab = new TabScenelist();
    ui->sequenceScrollArea->setWidget(scenes_tab);
}

SceneEditorWindow::~SceneEditorWindow()
{
    delete ui;
}

void SceneEditorWindow::on_actionSave_triggered()
{
    image_tab->save_data();
    viewpoint_tab->save_data();
    text_tab->save_data();
    scenes_tab->save_data();
}

