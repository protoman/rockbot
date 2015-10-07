#include "comboboxdelegate.h"


#include <QComboBox>
#include <QWidget>
#include <QModelIndex>
#include <QApplication>
#include <QString>

#include <iostream>

ComboBoxDelegate::ComboBoxDelegate(QObject *parent)
:QItemDelegate(parent)
{
  Items.push_back("Time (ms)");
  Items.push_back("Repear Number");
}


QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option , const QModelIndex &index) const
{
    QComboBox* editor = new QComboBox(parent);
    if (data_matrix.size() == 0) {
        for (unsigned int i = 0; i < Items.size(); ++i) {
            editor->addItem(Items[i].c_str());
        }
    } else {
        for (unsigned int i = 0; i < data_matrix.at(index.row()).size(); ++i) {
            editor->addItem(data_matrix.at(index.row()).at(i).c_str());
        }
    }
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = index.model()->data(index, Qt::EditRole).toUInt();
    comboBox->setCurrentIndex(value);
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = comboBox->currentIndex();
    model->setData(index, value, Qt::EditRole);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void ComboBoxDelegate::set_data_matrix(std::vector<std::vector<std::string> > data)
{
    data_matrix = data;
}






