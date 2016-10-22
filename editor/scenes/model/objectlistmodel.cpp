#include "objectlistmodel.h"
#include <iostream>

ObjectListModel::ObjectListModel(QObject *parent) : QAbstractTableModel(parent)
{
}


int ObjectListModel::rowCount(const QModelIndex &parent) const
{
    if (ScenesMediator::get_instance()->selected_scene >= ScenesMediator::get_instance()->scenes_list.size()) {
        return 0;
    }
    int n = 0;
    for (unsigned int i =0; i<SCENE_OBJECTS_MAX; i++) {
        if (ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[i].seek_n == -1) {
            break;
        }
        n++;
    }
    //std::cout << "ScenesMediator::get_instance()->selected_scene: " << ScenesMediator::get_instance()->selected_scene << ", scene_count: " << n << std::endl;
    return n;
}

int ObjectListModel::columnCount(const QModelIndex &parent) const
{
    return 5;
}

QVariant ObjectListModel::data(const QModelIndex &index, int role) const
{

    //std::cout << "+++++++++++++++ ObjectListModel::data - RUN #1 +++++++++++++++++++++++" << std::endl;

    int row = index.row();
    int col = index.column();

    //std::cout << "ObjectListModel::data - row: " << row << ", col: " << col << std::endl;

    if (ScenesMediator::get_instance()->selected_scene > ScenesMediator::get_instance()->scenes_list.size()-1) {
        std::cout << "ObjectListModel::data - LEAVE #1" << std::endl;
        return QVariant();
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        int type = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[row].type;
        int seek_n = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[row].seek_n;
        if (col == 0) {
            std::string name = std::string("NAME");
            if (type == CURRENT_FILE_FORMAT::SCENETYPE_CLEAR_AREA) {
                name = std::string(ScenesMediator::get_instance()->cleararea_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_MOVE_IMAGE) {
                name = std::string(ScenesMediator::get_instance()->image_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_MOVE_VIEWPOINT) {
                int list_size = ScenesMediator::get_instance()->viewpoint_list.size();
                if (list_size > seek_n) {
                    name = std::string(ScenesMediator::get_instance()->viewpoint_list.at(seek_n).name);
                }
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_PLAY_MUSIC) {
                name = std::string(ScenesMediator::get_instance()->playmusic_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_PLAY_SFX) {
                name = std::string(ScenesMediator::get_instance()->playsfx_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_SHOW_ANIMATION) {
                name = std::string(ScenesMediator::get_instance()->animation_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_SHOW_TEXT) {
                name = std::string(ScenesMediator::get_instance()->text_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_STOP_MUSIC) {
                name = std::string("STOP MUSIC");
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_SUBSCENE) {
                name = std::string(ScenesMediator::get_instance()->scenes_list.at(seek_n).name);
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_CLEAR_SCREEN) {
                name = std::string("CLEAR-SCREEN");
            }
            return QString(name.c_str());
        } else if (col == 1) {
            return QString::number(ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[row].delay_after);
        } else if (col == 2) {
            if (ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[row].repeat_type == 0) {
                return QString("Time (ms)");
            } else if (ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[row].repeat_type == 1){
                return QString("Repeat Number");
            } else {
                return QString("-Select-");
            }
        } else if (col == 3) {
            return QString::number(ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[row].repeat_value);
        } else if (col == 4) {
            return "";
        } else {
            return QString("Row%1, Column%2").arg(index.row() + 1).arg(index.column() +1);
        }
    } else if (role == Qt::CheckStateRole && col == 4) {
        if (ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[row].run_in_background == true) {
            return Qt::Checked;
        } else {
            return Qt::Unchecked;
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
                return QString("Item Name");
            case 1:
                return QString("After Delay");
            case 2:
                return QString("Repeat Type");
            case 3:
                return QString("Repeat Value");
            case 4:
                return QString("Run In Parallel");
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags ObjectListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return 0;

    Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;


    if (index.column() == 0) {
        int type = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[index.row()].type;
        if (type != CURRENT_FILE_FORMAT::SCENETYPE_CLEAR_SCREEN)         {
            result |= Qt::ItemIsEditable;
        }
    } else {
        result |= Qt::ItemIsEditable;
    }
    if (index.column() == 4) {
        result |= Qt::ItemIsUserCheckable;
    }

    return result;
}

bool ObjectListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int row = index.row();
    int col = index.column();

    if (index.isValid() && role == Qt::EditRole) {
        if (col == 0) {
            std::string name = value.toString().toStdString();
            int type = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[row].type;
            int seek_n = ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[row].seek_n;

            if (type == CURRENT_FILE_FORMAT::SCENETYPE_CLEAR_AREA) {
                sprintf(ScenesMediator::get_instance()->cleararea_list.at(seek_n).name, "%s", name.c_str());
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_MOVE_IMAGE) {
                sprintf(ScenesMediator::get_instance()->image_list.at(seek_n).name, "%s", name.c_str());
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_MOVE_VIEWPOINT) {
                sprintf(ScenesMediator::get_instance()->viewpoint_list.at(seek_n).name, "%s", name.c_str());
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_PLAY_MUSIC) {
                sprintf(ScenesMediator::get_instance()->playmusic_list.at(seek_n).name, "%s", name.c_str());
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_PLAY_SFX) {
                sprintf(ScenesMediator::get_instance()->playsfx_list.at(seek_n).name, "%s", name.c_str());
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_SHOW_ANIMATION) {
                sprintf(ScenesMediator::get_instance()->animation_list.at(seek_n).name, "%s", name.c_str());
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_SHOW_TEXT) {
                sprintf(ScenesMediator::get_instance()->text_list.at(seek_n).name, "%s", name.c_str());
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_STOP_MUSIC) {
                sprintf(ScenesMediator::get_instance()->cleararea_list.at(seek_n).name, "%s", name.c_str());
            } else if (type == CURRENT_FILE_FORMAT::SCENETYPE_SUBSCENE) {
                sprintf(ScenesMediator::get_instance()->scenes_list.at(seek_n).name, "%s", name.c_str());
            } else {
                std::cout << "Invalid object to set name" << std::endl;
                return false;
            }
        } else if (col == 1) {
            ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[row].delay_after = value.toInt();
        } else if (col == 2) {
            ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[row].repeat_type = value.toInt();
        } else if (col == 3) {
            ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[row].repeat_value = value.toInt();
        } else if (col == 4) {
            ScenesMediator::get_instance()->scenes_list.at(ScenesMediator::get_instance()->selected_scene).objects[row].run_in_background = value.toBool();
        }
        emit(dataChanged(index, index));
        return true;
    } else if (role == Qt::CheckStateRole && col == 4) {
        if ((Qt::CheckState)value.toInt() == Qt::Checked) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

void ObjectListModel::update()
{
    QModelIndex topLeft = index(0, 0);
    QModelIndex bottomRight = index(rowCount()-1, columnCount()-1);


    //std::cout << "%%%%%%%%%%%%%% ObjectListModel::update - RUN, row: " << (rowCount()-1) << " %%%%%%%%%%%%%%" << std::endl;

    emit dataChanged ( topLeft, bottomRight );
    emit layoutChanged();
}
