#include "combolistdelegate.h"
#include "mediator.h"

ComboListDelegate::ComboListDelegate(QObject *parent, std::map<int, std::vector<std::string> > set_data_map) : QItemDelegate(parent)
{
    data_map = set_data_map;
}

QWidget *ComboListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox* editor = new QComboBox(parent);
    unsigned int row = index.row();

    std::map<int, std::vector<std::string> >::const_iterator iter = data_map.find(row);
    int max = iter->second.size();

    editor->addItem(Mediator::get_instance()->combobox_select_string.c_str());
    for (unsigned int i = 0; i < max; ++i) {
        editor->addItem(iter->second.at(i).c_str());
    }
    connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));

    return editor;
}

void ComboListDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = index.model()->data(index, Qt::EditRole).toUInt();
    comboBox->setCurrentIndex(value);
}

void ComboListDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    QString value = comboBox->currentText();
    model->setData(index, value, Qt::EditRole);
}

void ComboListDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void ComboListDelegate::onCurrentIndexChanged(int index)
{
    QComboBox* cb = static_cast<QComboBox*>(sender());
    if (cb) {
      emit commitData(cb);
    }
}



