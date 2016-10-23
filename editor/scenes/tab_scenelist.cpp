#include "tab_scenelist.h"
#include "ui_tab_scenelist.h"
#include "scenes/comboboxdelegate.h"

#include <QMessageBox>

extern std::string GAMEPATH;
extern std::string GAMENAME;

TabScenelist::TabScenelist(QWidget *parent) : QDialog(parent), ui(new Ui::ScenesList), model_scenes(this)
{
    ui->setupUi(this);
    ui->object_listView->setModel(&model_objects);
    ui->scenes_tableView->setModel(&model_scenes);
    ui->scenes_tableView->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->scenes_tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->scenes_tableView->setAlternatingRowColors(true);
    ui->scenes_tableView->setStyleSheet("alternate-background-color: #dafeff;background-color: #dddddd;");

    ComboBoxDelegate* delegate = new ComboBoxDelegate(this);
    ui->scenes_tableView->setItemDelegateForColumn(2, delegate);
    /// @TODO: find a way that works for both Qt4 and 5 - http://stackoverflow.com/questions/17535563/how-to-get-a-qtableview-to-fill-100-of-the-width

#if QT_VERSION >= 0x050000
    ui->scenes_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // Qt5
#else
    ui->scenes_tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch); // Qt4
#endif

    data_loading = true;

    ScenesMediator::get_instance()->scenes_list = fio.load_scenes();

    fill_data();
    data_loading = false;
}

TabScenelist::~TabScenelist()
{
    delete ui;
}

// add a new scene
void TabScenelist::on_addScene_button_clicked()
{
    CURRENT_FILE_FORMAT::file_scene_list new_scene;
    sprintf(new_scene.name, "%s%d", "Scene List #", ScenesMediator::get_instance()->scenes_list.size()+1);
    ScenesMediator::get_instance()->scenes_list.push_back(new_scene);
    ui->sceneSelector->addItem(QString(new_scene.name));
    if (ScenesMediator::get_instance()->scenes_list.size() == 1) {
        change_fields_enabled(true);
    }
    ui->sceneSelector->setCurrentIndex(ScenesMediator::get_instance()->scenes_list.size()-1);

}


void TabScenelist::fill_data()
{
    ui->sceneSelector->clear();

    std::cout << "fill_data::ScenesMediator::get_instance()->scenes_list.size(): " << ScenesMediator::get_instance()->scenes_list.size() << std::endl;

    for (int i=0; i<ScenesMediator::get_instance()->scenes_list.size(); i++) {
        ui->sceneSelector->addItem(QString(ScenesMediator::get_instance()->scenes_list.at(i).name));
    }
    // combox zero position is clear screen, so just add one item
    QStringList list;
    list.append(QString("Clear"));
    model_objects.setStringList(list);
}

void TabScenelist::save_data()
{
    fio.save_scenes(ScenesMediator::get_instance()->scenes_list);
}

void TabScenelist::reload()
{
    data_loading = true;
    ScenesMediator::get_instance()->scenes_list = fio.load_scenes();
    fill_data();
    data_loading = false;
}

void TabScenelist::change_fields_enabled(bool value)
{
    ui->sceneSelector->setEnabled(value);
    ui->sceneTypeSelector->setEnabled(value);
    ui->addButton->setEnabled(value);
    ui->removeButton->setEnabled(value);
}

void TabScenelist::on_sceneTypeSelector_currentIndexChanged(int index)
{
    QStringList list;
    if (index == CURRENT_FILE_FORMAT::SCENETYPE_CLEAR_SCREEN) {
        list.append(QString("Clear"));
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_CLEAR_AREA) {
        for (unsigned int i=0; i<ScenesMediator::get_instance()->cleararea_list.size(); i++) {
            list.append(QString(ScenesMediator::get_instance()->cleararea_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_MOVE_IMAGE) {
        for (unsigned int i=0; i<ScenesMediator::get_instance()->image_list.size(); i++) {
            list.append(QString(ScenesMediator::get_instance()->image_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_MOVE_VIEWPOINT) {
        for (unsigned int i=0; i<ScenesMediator::get_instance()->viewpoint_list.size(); i++) {
            list.append(QString(ScenesMediator::get_instance()->viewpoint_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_SHOW_ANIMATION) {
        for (unsigned int i=0; i<ScenesMediator::get_instance()->animation_list.size(); i++) {
            list.append(QString(ScenesMediator::get_instance()->animation_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_PLAY_SFX) {
        for (unsigned int i=0; i<ScenesMediator::get_instance()->playsfx_list.size(); i++) {
            list.append(QString(ScenesMediator::get_instance()->playsfx_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_PLAY_MUSIC) {
        for (unsigned int i=0; i<ScenesMediator::get_instance()->playmusic_list.size(); i++) {
            list.append(QString(ScenesMediator::get_instance()->playmusic_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_STOP_MUSIC) {
        // nothing to do
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_SHOW_TEXT) {
        for (unsigned int i=0; i<ScenesMediator::get_instance()->text_list.size(); i++) {
            list.append(QString(ScenesMediator::get_instance()->text_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_SUBSCENE) {
        for (unsigned int i=0; i<ScenesMediator::get_instance()->scenes_list.size(); i++) {
            //if (i != ui->sceneSelector->currentIndex()) {
                list.append(QString(ScenesMediator::get_instance()->scenes_list.at(i).name));
            //}
        }
    }
    model_objects.setStringList(list);
}

// add a item into the scene
void TabScenelist::on_addButton_clicked()
{
    if (ScenesMediator::get_instance()->scenes_list.size() == 0) {
        return;
    }

    int n = ui->sceneSelector->currentIndex();


    QModelIndexList selectedList = ui->object_listView->selectionModel()->selectedRows();

    if (selectedList.count() == 0) {
        return;
    }

    int seek_n = selectedList.at(0).row();

    // can't add sub-scene equal to the current scene
    if (n == seek_n && ui->sceneTypeSelector->currentIndex() == CURRENT_FILE_FORMAT::SCENETYPE_SUBSCENE) {
        QMessageBox msgBox;
        msgBox.setText("Can't add scene itself as sub-scene of itself. This would create an infinite loop.");
        msgBox.exec();
        return;
    }


    // search for a free slot
    for (unsigned int i = 0; i<SCENE_OBJECTS_MAX; i++) {
        if (ScenesMediator::get_instance()->scenes_list.at(n).objects[i].seek_n == -1) {
            ScenesMediator::get_instance()->scenes_list.at(n).objects[i].seek_n = seek_n;
            ScenesMediator::get_instance()->scenes_list.at(n).objects[i].type = ui->sceneTypeSelector->currentIndex();
            std::cout << "ADDED at row["  << i << "], ScenesMediator::get_instance()->scenes_list.size: " << (ScenesMediator::get_instance()->scenes_list.size()) << std::endl;

            ui->scenes_tableView->setModel(&model_scenes);
            model_scenes.update();
            break;
        }
    }
}


void TabScenelist::on_sceneSelector_currentIndexChanged(int index)
{
    ScenesMediator::get_instance()->selected_scene = index;
    if (index != -1) {
        ui->name_lineEdit->setText(ScenesMediator::get_instance()->scenes_list.at(index).name);
        model_scenes.update();
    }
}

void TabScenelist::on_removeButton_clicked()
{
    QModelIndexList selectedList = ui->scenes_tableView->selectionModel()->selectedRows();

    if (selectedList.count() == 0) {
        return;
    }

    for (int i=0; i<selectedList.count(); i++) {
        int selectedRow = selectedList.at(i).row();
        //QMessageBox::information(this,"", QString::number(selectedRow));
        // move all above rows 1 level down
        for (int j=selectedRow+1; j<SCENE_OBJECTS_MAX; j++) {
            ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[j-1].seek_n = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[j].seek_n;
            ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[j-1].type = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[j].type;
            ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[j-1].delay_after = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[j].delay_after;
            ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[j-1].repeat_type = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[j].repeat_type;
            ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[j-1].repeat_value = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[j].repeat_value;
        }
        break;
    }
    // empty last line, as there is none above to move down
    ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[SCENE_OBJECTS_MAX-1].seek_n = -1;
    ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[SCENE_OBJECTS_MAX-1].type = -1;
    ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[SCENE_OBJECTS_MAX-1].delay_after = 100;
    ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[SCENE_OBJECTS_MAX-1].repeat_type = 0;
    ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[SCENE_OBJECTS_MAX-1].repeat_value = 0;

    ui->scenes_tableView->setModel(&model_scenes);
    model_scenes.update();
}

void TabScenelist::on_pushButton_clicked()
{
    QString file = QString(GAMEPATH.c_str()) + QString("scenesviewer");
#ifdef WIN32
    file += QString(".exe");
#endif
    file += QString(" --gamename \"") + QString(GAMENAME.c_str()) + QString("\"") + QString(" --scenenumber ") + QString::number(ui->sceneSelector->currentIndex());
    std::cout << ">>> EXEC: file: '" << file.toStdString() << "'." << std::endl;
    process.start(file);
}

void TabScenelist::on_name_lineEdit_textChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).name, "%s", arg1.toStdString().c_str());
}

void TabScenelist::on_up_pushButton_clicked()
{
    QModelIndexList selectedList = ui->scenes_tableView->selectionModel()->selectedRows();
    // no line selected, do nothing
    if (selectedList.count() == 0) {
        return;
    }
    int selected_row = selectedList.at(0).row();
    // can't move up first row
    if (selected_row == 0) {
        return;
    }

    CURRENT_FILE_FORMAT::file_scene_object temp = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[selected_row-1];
    ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[selected_row-1] = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[selected_row];
    ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[selected_row] = temp;

    QModelIndex next_index = ui->scenes_tableView->model()->index(selected_row - 1, 0);
    ui->scenes_tableView->setCurrentIndex(next_index);

    model_scenes.update();
}

void TabScenelist::on_down_pushButton_clicked()
{
    QModelIndexList selectedList = ui->scenes_tableView->selectionModel()->selectedRows();
    // no line selected, do nothing
    if (selectedList.count() == 0) {
        return;
    }
    int selected_row = selectedList.at(0).row();
    // can't move down last row
    if (selected_row == ui->scenes_tableView->selectionModel()->children().count()-1) {
        return;
    }

    CURRENT_FILE_FORMAT::file_scene_object temp = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[selected_row+1];
    ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[selected_row+1] = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[selected_row];
    ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[selected_row] = temp;

    QModelIndex next_index = ui->scenes_tableView->model()->index(selected_row + 1, 0);
    ui->scenes_tableView->setCurrentIndex(next_index);

    model_scenes.update();
}
