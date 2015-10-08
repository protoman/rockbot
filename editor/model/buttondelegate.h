#ifndef BUTTONDELEGATE_H
#define BUTTONDELEGATE_H

#include <QItemDelegate>
#include <QPushButton>
#include <QSignalMapper>
#include <QEvent>

class ButtonDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ButtonDelegate(QObject *parent, std::vector<std::string> set_dir_list, std::map<int, std::vector<std::string>> set_data_map);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:

public slots:
    void onButtonClicked(int);

private:
    std::vector<std::string> dir_list;
    std::map<int, std::vector<std::string>> data_map;
    QStyle::State  _state;
    QObject *_parent;
    QSignalMapper *signal_mapper;


};

#endif // BUTTONDELEGATE_H
