#ifndef STRINGEDITMODEL_H
#define STRINGEDITMODEL_H

#include <QAbstractTableModel>
#include <string>
#include <vector>

#include "file/v4/file_strings.h"

enum e_PICK_MODES {
    pick_mode_edit,
    pick_mode_common,
    pick_mode_scenes,
    pick_mode_count
};

class StringEditModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    StringEditModel(QObject *parent);
public:
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    void set_data(std::vector<std::string> data);
    std::vector<std::string> get_data();
    void add_line();
    void set_pick_mode(int mode);
    void update();

signals:

private:
    QObject *_parent;
    std::vector<std::string> string_list;
    int pick_mode;

};

#endif // STRINGEDITMODEL_H
