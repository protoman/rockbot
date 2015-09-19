#ifndef SCENEEDITORWINDOW_H
#define SCENEEDITORWINDOW_H

#include <QMainWindow>
#include "scenes/sceneslist.h"
#include "scenes/tab_text.h"
#include "scenes/tab_image.h"
#include "scenes/tab_viewpoint.h"

namespace Ui {
class SceneEditorWindow;
}

class SceneEditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SceneEditorWindow(QWidget *parent = 0);
    ~SceneEditorWindow();

private slots:
    void on_actionSave_triggered();

    void on_pushButton_clicked();
    void on_play_state_changed(bool state);

private:
    Ui::SceneEditorWindow *ui;
    // tab elements
    TabText* text_tab;
    tab_image* image_tab;
    tab_viewpoint* viewpoint_tab;


};

#endif // SCENEEDITORWINDOW_H
