#include "tab_scenelist.h"
#include "ui_tab_scenelist.h"
#include "scenes/comboboxdelegate.h"

#include <QMessageBox>

TabScenelist::TabScenelist(QWidget *parent) : QDialog(parent), ui(new Ui::ScenesList), model_scenes(this)
{
    ui->setupUi(this);
    mediator = ScenesMediator::get_instance();
    ui->object_listView->setModel(&model_objects);
    ui->scenes_tableView->setModel(&model_scenes);

    ComboBoxDelegate* delegate = new ComboBoxDelegate(this);
    ui->scenes_tableView->setItemDelegateForColumn(2, delegate);
    ui->scenes_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    data_loading = true;

    mediator->scenes_list = fio.load_scenes();

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
    sprintf(new_scene.name, "%s%d", "Scene List #", mediator->scenes_list.size()+1);
    mediator->scenes_list.push_back(new_scene);
    ui->sceneSelector->addItem(QString(new_scene.name));
    if (mediator->scenes_list.size() == 1) {
        change_fields_enabled(true);
    }
    ui->sceneSelector->setCurrentIndex(mediator->scenes_list.size()-1);

}


void TabScenelist::fill_data()
{
    ui->sceneSelector->clear();
    for (int i=0; i<mediator->scenes_list.size(); i++) {
        ui->sceneSelector->addItem(QString(mediator->scenes_list.at(i).name));
    }
}

void TabScenelist::save_data()
{
    fio.save_scenes(mediator->scenes_list);
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
        // nothing to do
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_CLEAR_AREA) {
        for (unsigned int i=0; i<mediator->cleararea_list.size(); i++) {
            list.append(QString(mediator->cleararea_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_MOVE_IMAGE) {
        for (unsigned int i=0; i<mediator->image_list.size(); i++) {
            list.append(QString(mediator->image_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_MOVE_VIEWPOINT) {
        for (unsigned int i=0; i<mediator->viewpoint_list.size(); i++) {
            list.append(QString(mediator->viewpoint_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_SHOW_ANIMATION) {
        for (unsigned int i=0; i<mediator->animation_list.size(); i++) {
            list.append(QString(mediator->animation_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_PLAY_SFX) {
        for (unsigned int i=0; i<mediator->playsfx_list.size(); i++) {
            list.append(QString(mediator->playsfx_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_PLAY_MUSIC) {
        for (unsigned int i=0; i<mediator->playmusic_list.size(); i++) {
            list.append(QString(mediator->playmusic_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_STOP_MUSIC) {
        // nothing to do
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_SHOW_TEXT) {
        for (unsigned int i=0; i<mediator->text_list.size(); i++) {
            list.append(QString(mediator->text_list.at(i).name));
        }
    } else if (index == CURRENT_FILE_FORMAT::SCENETYPE_SUBSCENE) {
        for (unsigned int i=0; i<mediator->scenes_list.size(); i++) {
            if (i != ui->sceneSelector->currentIndex()) {
                list.append(QString(mediator->scenes_list.at(i).name));
            }
        }
    }
    model_objects.setStringList(list);
}

// add a item into the scene
void TabScenelist::on_addButton_clicked()
{
    if (mediator->scenes_list.size() == 0) {
        return;
    }
    int n = ui->sceneSelector->currentIndex();
    // number of the selected line in objects list
    QModelIndex listSelection = ui->object_listView->currentIndex();
    int seek_n = listSelection.row();
    // search for a free slot
    for (unsigned int i = 0; i<SCENE_OBJECTS_N; i++) {
        if (mediator->scenes_list.at(n).objects[i].seek_n == -1) {
            mediator->scenes_list.at(n).objects[i].seek_n = seek_n;
            mediator->scenes_list.at(n).objects[i].type = ui->sceneTypeSelector->currentIndex();
            std::cout << "ADDED at row["  << i << "], mediator->scenes_list.size: " << (mediator->scenes_list.size()) << std::endl;

            ui->scenes_tableView->setModel(&model_scenes);
            model_scenes.update();
            break;
        }
    }
}


void TabScenelist::on_sceneSelector_currentIndexChanged(int index)
{
    mediator->selected_scene = index;
    model_scenes.update();
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
        for (int j=selectedRow+1; j<SCENE_OBJECTS_N; j++) {
            mediator->scenes_list.at(mediator->selected_scene).objects[j-1].seek_n = mediator->scenes_list.at(mediator->selected_scene).objects[j].seek_n;
            mediator->scenes_list.at(mediator->selected_scene).objects[j-1].type = mediator->scenes_list.at(mediator->selected_scene).objects[j].type;
            mediator->scenes_list.at(mediator->selected_scene).objects[j-1].delay_after = mediator->scenes_list.at(mediator->selected_scene).objects[j].delay_after;
            mediator->scenes_list.at(mediator->selected_scene).objects[j-1].repeat_type = mediator->scenes_list.at(mediator->selected_scene).objects[j].repeat_type;
            mediator->scenes_list.at(mediator->selected_scene).objects[j-1].repeat_value = mediator->scenes_list.at(mediator->selected_scene).objects[j].repeat_value;
        }
        break;
    }
    // empty last line, as there is none above to move down
    mediator->scenes_list.at(mediator->selected_scene).objects[SCENE_OBJECTS_N-1].seek_n = -1;
    mediator->scenes_list.at(mediator->selected_scene).objects[SCENE_OBJECTS_N-1].type = -1;
    mediator->scenes_list.at(mediator->selected_scene).objects[SCENE_OBJECTS_N-1].delay_after = 100;
    mediator->scenes_list.at(mediator->selected_scene).objects[SCENE_OBJECTS_N-1].repeat_type = 0;
    mediator->scenes_list.at(mediator->selected_scene).objects[SCENE_OBJECTS_N-1].repeat_value = 0;

    ui->scenes_tableView->setModel(&model_scenes);
    model_scenes.update();

}
