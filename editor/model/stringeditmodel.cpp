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

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (col == 0) {
            QString res = QString::number(row);
            return res;
        } else if (col == 1) {
            QString res = QString(string_list.at(index.row()).c_str());
            std::cout << "StringEditModel::col0[" << res.toStdString() << "]" << std::endl;
            return res;
        }
    }

    return QVariant();

}

Qt::ItemFlags StringEditModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return 0;

    Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (pick_mode_enabled == false && index.column() == 1) {
        result |= Qt::ItemIsEditable;
    }

    return result;
}

bool StringEditModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int row = index.row();
    int col = index.column();

    std::string str_value = value.toString().toStdString();
    if (col == 1) {
        string_list.at(row) = str_value;
        //emit selected_image_changed(list_directories.at(row) + std::string("/") + str_value);
    }

    return true;
}

void StringEditModel::set_data(std::vector<std::string> data)
{
    string_list = data;
}

void StringEditModel::set_pick_mode(bool pick_mode)
{
    pick_mode_enabled = pick_mode;
}

