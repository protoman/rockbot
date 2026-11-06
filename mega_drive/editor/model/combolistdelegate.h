#ifndef COMBOLISTDELEGATE_H
#define COMBOLISTDELEGATE_H

#include <QObject>
#include <QItemDelegate>
#include <QComboBox>

class ComboListDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ComboListDelegate(QObject *parent, std::map<int, std::vector<std::string> > set_data_map);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void set_row_data(int row, std::vector<std::string> data);

public slots:
    void onCurrentIndexChanged(int);

private:
    std::map<int, std::vector<std::string> > data_map;
};

#endif // COMBOLISTDELEGATE_H
