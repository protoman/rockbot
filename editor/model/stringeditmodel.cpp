#include "stringeditmodel.h"

StringEditModel::StringEditModel(QObject *parent)
{
    _parent = parent;
    pick_mode_enabled = false;
}

int StringEditModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

int StringEditModel::rowCount(const QModelIndex &parent) const
{
    return string_list.size();
}

QVariant StringEditModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("Name");
            case 1:
                return QString("Value");
            }
        }
    }
    return QVariant();

}

QVariant StringEditModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    // out of scope value
    if (row >= string_list.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (col == 0) {
            return QString(string_list.at(row).name);
        } else if (col == 1) {
            return QString(string_list.at(row).value);
        } else {
            return QString("Row%1, Column%2").arg(index.row() + 1).arg(index.column() +1);
        }
    }

    return QVariant();

}

Qt::ItemFlags StringEditModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return 0;

    Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (pick_mode_enabled == false) {
        result |= Qt::ItemIsEditable;
    }


    return result;
}

bool StringEditModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int row = index.row();
    int col = index.column();

    std::string str_value = value.toString().toStdString();
    if (col == 0) {
        string_list.at(row).set_name(str_value);
        //emit selected_image_changed(list_directories.at(row) + std::string("/") + str_value);
    } else if (col == 1) {
        string_list.at(row).set_value(str_value);
    }

    return true;
}

void StringEditModel::set_data(std::vector<format_v4::st_file_common_string> data)
{
    string_list = data;
}

void StringEditModel::set_pick_mode(bool pick_mode)
{
    pick_mode_enabled = pick_mode;
}

