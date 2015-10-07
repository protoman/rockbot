#include "buttondelegate.h"

#include <iostream>
#include <QEvent>
#include <QMouseEvent>
#include <QFileDialog>

ButtonDelegate::ButtonDelegate(QObject *parent, std::map<int, std::vector<std::string> > set_data_map)
{
    data_map = set_data_map;
    _state =  QStyle::State_Enabled;
    _parent = parent;
}

QWidget *ButtonDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
        QPushButton* editor = new QPushButton(parent);
        unsigned int row = index.row();
        editor->setText("Import");

        return editor;
}

void ButtonDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QPushButton *button = static_cast<QPushButton*>(editor);
    unsigned int row = index.row();

}

void ButtonDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QPushButton *button = static_cast<QPushButton*>(editor);
    //QString value = comboBox->currentText();
    //model->setData(index, value, Qt::EditRole);
}

void ButtonDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

bool ButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        event->ignore();
        std::cout << "MOUSE-RELEASED" << std::endl;

        QString filename = QFileDialog::getOpenFileName((QWidget*)_parent, tr("Open Image File 1"), "/home", tr("Image Files (*.png)"));
        //ui->File1Path->setText(file1Name);

        std::cout << ">>> filename: " << filename.toStdString() << std::endl;

        return false;
    } else {
        return QItemDelegate::editorEvent(event, model, option, index);
    }
}

