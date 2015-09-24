#ifndef OBJECTLISTMODEL_H
#define OBJECTLISTMODEL_H

#include <QAbstractTableModel>
#include "scenes/scenesmediator.h"

class ObjectListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ObjectListModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void update();

private:
    ScenesMediator *mediator;
};

#endif // OBJECTLISTMODEL_H
