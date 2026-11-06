#include "image_directories_model.h"

#include <QFileDialog>

ImageDirectoriesModel::ImageDirectoriesModel(QObject *parent)
{
    list_directories.push_back("/images/");
    list_directories.push_back("/images/animations");
    list_directories.push_back("/images/backgrounds");
    list_directories.push_back("/images/faces");
    list_directories.push_back("/images/map_backgrounds");
    list_directories.push_back("/images/projectiles");
    list_directories.push_back("/images/scenes");
    list_directories.push_back("/images/scenes/animations");
    list_directories.push_back("/images/sprites");
    list_directories.push_back("/images/sprites/enemies");
    list_directories.push_back("/images/sprites/objects");
    list_directories.push_back("/images/tilesets");
    list_directories.push_back("/images/tilesets/anim");
    for (int i=0; i<list_directories.size(); i++) {
        list_dir_selected_value.push_back("- Select -");
    }
    _parent = parent;
}

int ImageDirectoriesModel::rowCount(const QModelIndex &parent) const
{
    return list_directories.size();
}

int ImageDirectoriesModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

QVariant ImageDirectoriesModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    // out of scope value
    if (row >= list_directories.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (col == 0) {
            return QString(list_directories.at(row).c_str());
        } else if (col == 1) {
            std::string value = list_dir_selected_value.at(row);
            return QString(value.c_str());
        } else if (col == 2) {
            return QString("Import...");
        } else {
            return QString("Row%1, Column%2").arg(index.row() + 1).arg(index.column() +1);
        }
    }

    return QVariant();
}

QVariant ImageDirectoriesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("Directory Name");
            case 1:
                return QString("Preview");
            case 2:
                return QString("Import");
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags ImageDirectoriesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return 0;

    Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() != 0) {
        result |= Qt::ItemIsEditable;
    }

    return result;
}

bool ImageDirectoriesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int row = index.row();
    int col = index.column();


    if (col == 1) {
        std::string str_value = value.toString().toStdString();
        list_dir_selected_value.at(row) = str_value;
        emit selected_image_changed(list_directories.at(row) + std::string("/") + str_value);
    }

    return true;

}

void ImageDirectoriesModel::update()
{

}

std::vector<std::string> ImageDirectoriesModel::get_directory_list()
{
    return list_directories;
}

