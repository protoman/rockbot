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

    sfx_tab = new TabSfx();
    ui->tab_sfx_scrollArea->setWidget(sfx_tab);

    music_tab = new TabMusic();
    ui->tab_music_scrollArea->setWidget(music_tab);

    cleararea_tab = new TabClearArea();
    ui->tab_cleararea_scrollArea->setWidget(cleararea_tab);

    animation_tab = new TabAnimation();
    ui->tabAnimation_scrollArea->setWidget(animation_tab);
}

SceneEditorWindow::~SceneEditorWindow()
{
    delete ui;
}

void SceneEditorWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Rockbot Editor :: Movie Editor", tr("Save data before leaving?\n"), QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        save();
    }
    event->accept();
}

void SceneEditorWindow::reload()
{
    image_tab->reload();
    viewpoint_tab->reload();
    text_tab->reload();
    scenes_tab->reload();
    sfx_tab->reload();
    music_tab->reload();
    cleararea_tab->reload();
    animation_tab->reload();
}

void SceneEditorWindow::on_actionSave_triggered()
{
    save();
}

void SceneEditorWindow::save()
{
    image_tab->save_data();
    viewpoint_tab->save_data();
    text_tab->save_data();
    scenes_tab->save_data();
    sfx_tab->save_data();
    music_tab->save_data();
    cleararea_tab->save_data();
    animation_tab->save_data();
}

