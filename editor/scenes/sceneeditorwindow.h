#ifndef SCENEEDITORWINDOW_H
#define SCENEEDITORWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QMessageBox>
#include <QCloseEvent>

#include "scenes/tab_scenelist.h"
#include "scenes/tab_text.h"
#include "scenes/tab_image.h"
#include "scenes/tab_viewpoint.h"
#include "scenes/tab_scenelist.h"
#include "scenes/tab_sfx.h"
#include "scenes/tab_music.h"
#include "scenes/tab_cleararea.h"
#include "scenes/tab_animation.h"

namespace Ui {
class SceneEditorWindow;
}

class SceneEditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SceneEditorWindow(QWidget *parent = 0);
    ~SceneEditorWindow();
    void closeEvent (QCloseEvent *event);
    void reload();

signals:
    void scenes_editor_window_closed();

private slots:
    void on_actionSave_triggered();

    void on_actionPlay_Movie_triggered();

private:
    void save();

private:
    Ui::SceneEditorWindow *ui;
    // tab elements
    TabText* text_tab;
    tab_image* image_tab;
    tab_viewpoint* viewpoint_tab;
    TabScenelist* scenes_tab;
    TabSfx* sfx_tab;
    TabMusic* music_tab;
    TabClearArea* cleararea_tab;
    TabAnimation* animation_tab;
    QProcess process;
};

#endif // SCENEEDITORWINDOW_H
