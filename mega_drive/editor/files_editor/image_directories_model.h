#ifndef IMAGEDIRECTORIESMODEL_H
#define IMAGEDIRECTORIESMODEL_H

#include <QAbstractTableModel>
#include <vector>
#include "scenes/scenesmediator.h"


class ImageDirectoriesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ImageDirectoriesModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    void update();
    std::vector<std::string> get_directory_list();

signals:
    void selected_image_changed(std::string filename);

private:
    std::vector<std::string> list_directories;
    std::vector<std::string> list_dir_selected_value;           // row x value
    QObject *_parent;

};

#endif // IMAGEDIRECTORIESMODEL_H
