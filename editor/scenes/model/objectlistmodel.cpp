#include "objectlistmodel.h"
#include <iostream>

ObjectListModel::ObjectListModel(QObject *parent) :QAbstractTableModel(parent)
{
    mediator = ScenesMediator::get_instance();
}


int ObjectListModel::rowCount(const QModelIndex &parent) const
{
    if (mediator->selected_scene >= mediator->scenes_list.size()) {
        return 0;
    }
    std::cout << "mediator->selected_scene: " << mediator->selected_scene << ", mediator->scenes_list.size(): " << (mediator->scenes_list.size()) << std::endl;
    int n = 0;
    for (unsigned int i =0; i<SCENE_OBJECTS_N; i++) {
        if (mediator->scenes_list.at(mediator->selected_scene).objects[i].seek_n == -1) {
            break;
        }
        n++;
    }
    return n;
}

int ObjectListModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

QVariant ObjectListModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if (mediator->selected_scene > mediator->scenes_list.size()-1) {
        std::cout << "ObjectListModel::data - LEAVE #1" << std::endl;
        return QVariant();
    }
    if (role == Qt::DisplayRole) {
        if (col == 0) {
            int type = mediator->scenes_list.at(mediator->selected_scene).objects[row].type;
            int seek_n = mediator->scenes_list.at(mediator->selected_scene).objects[row].seek_n;
            std::string name = std::string("NAME");
            if (type == CURRENT_FILE_FORMAT::SCENETYPE_CLEAR_AREA) {
                name = std::string(mediator->cleararea_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_MOVE_IMAGE) {
                name = std::string(mediator->image_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_MOVE_VIEWPOINT) {
                name = std::string(mediator->viewpoint_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_PLAY_MUSIC) {
                name = std::string(mediator->playmusic_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_PLAY_SFX) {
                name = std::string(mediator->playsfx_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_SHOW_ANIMATION) {
                name = std::string(mediator->animation_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_SHOW_TEXT) {
                name = std::string(mediator->text_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_STOP_MUSIC) {
                name = std::string("STOP MUSIC");
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_SUBSCENE) {
                name = std::string(mediator->scenes_list.at(seek_n).name);
            }
            return QString(name.c_str());
        } else {
            return QString("Row%1, Column%2").arg(index.row() + 1).arg(index.column() +1);
        }
    }



    return QVariant();
}

QVariant ObjectListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("Object Name");
            case 1:
                return QString("Delay for Next");
            case 2:
                return QString("Repeat Type");
            case 3:
                return QString("Repeat Value");
            }
        }
    }
    return QVariant();
}

void ObjectListModel::update()
{
    QModelIndex topLeft = index(0, 0);
    QModelIndex bottomRight = index(rowCount()-1, columnCount()-1);
    emit dataChanged ( topLeft, bottomRight );
}
