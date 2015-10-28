#ifndef SCENESLIST_H
#define SCENESLIST_H

#include <QDialog>
#include <QStringListModel>
#include <QStringList>
#include <QProcess>

#include "scenes/scenesmediator.h"
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
    void save_data();
    void reload();

private:
    void fill_data();
    void change_fields_enabled(bool value);

private slots:
    void on_sceneTypeSelector_currentIndexChanged(int index);

    void on_addButton_clicked();

    void on_addScene_button_clicked();

    void on_sceneSelector_currentIndexChanged(int index);

    void on_removeButton_clicked();

    void on_pushButton_clicked();

    void on_name_lineEdit_textChanged(const QString &arg1);

    void on_up_pushButton_clicked();

    void on_down_pushButton_clicked();

private:
    Ui::ScenesList *ui;
    QStringListModel model_objects;
    ObjectListModel model_scenes;
    bool data_loading;
    CURRENT_FILE_FORMAT::fio_scenes fio;
    QProcess process;
};

#endif // SCENESLIST_H
