#ifndef SCENESLIST_H
#define SCENESLIST_H

#include <QDialog>
#include <QStringListModel>
#include <QStringList>
#include "scenes/scenesmediator.h"
#include "file/v3/3_0_1/file_scene.h"
#include "file/fio_scenes.h"
#include "model/objectlistmodel.h"

namespace Ui {
class ScenesList;
}

class TabScenelist : public QDialog
{
    Q_OBJECT

public:
    explicit TabScenelist(QWidget *parent = 0);
    ~TabScenelist();


private:
    void fill_data();
    void save_data();
    void change_fields_enabled(bool value);

private slots:
    void on_sceneTypeSelector_currentIndexChanged(int index);

    void on_addButton_clicked();

    void on_addScene_button_clicked();

private:
    Ui::ScenesList *ui;
    QStringListModel model_objects;
    ObjectListModel model_scenes;
    ScenesMediator *mediator;
    bool data_loading;
    CURRENT_FILE_FORMAT::fio_scenes fio;
};

#endif // SCENESLIST_H
