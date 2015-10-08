#include "buttondelegate.h"

#include <iostream>
#include <QEvent>
#include <QMouseEvent>
#include <QFileDialog>

extern std::string FILEPATH;

ButtonDelegate::ButtonDelegate(QObject *parent, std::vector<std::string> set_dir_list, std::map<int, std::vector<std::string> > set_data_map)
{
    dir_list = set_dir_list;
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
        QFileInfo file_info(filename);
        std::cout << ">>> filename: " << filename.toStdString() << std::endl;
        int row = index.row();
        if (row >= dir_list.size()) {
            std::cout << ">>> ButtonDelegate::editorEvent::ERROR: row[" << row << "] not found in data_map." << std::endl;
        } else {
            std::string dest_path = FILEPATH + dir_list.at(row) + std::string("/") + file_info.fileName().toStdString();
            std::cout << "ButtonDelegate::editorEvent::dest_path: " << dest_path << std::endl;
            if (QFile::copy(filename, QString(dest_path.c_str())) == false) {
                std::cout << "ButtonDelegate::editorEvent::ERROR: Can't copy file." << std::endl;
            }
        }

        return false;
    } else {
        return QItemDelegate::editorEvent(event, model, option, index);
    }
}


