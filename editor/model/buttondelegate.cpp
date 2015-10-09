#include "buttondelegate.h"

#include <iostream>
#include <QEvent>
#include <QMouseEvent>
#include <QFileDialog>
#include <QApplication>
#include <QDesktopServices>

extern std::string FILEPATH;

ButtonDelegate::ButtonDelegate(QObject *parent, std::vector<std::string> set_dir_list, std::map<int, std::vector<std::string> > set_data_map, QAbstractTableModel* model) : signal_mapper(new QSignalMapper(this))
{
    _model = model;
    dir_list = set_dir_list;
    data_map = set_data_map;
    _state =  QStyle::State_Enabled;
    _parent = parent;
    operation_mode = 0;
    connect(signal_mapper, SIGNAL(mapped(const int &)), this, SLOT(onButtonClicked(const int &)));
}

QWidget *ButtonDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
        QPushButton* editor = new QPushButton(parent);
        if (operation_mode == 0) {
            editor->setText("Import");
        } else if (operation_mode == 0) {
            editor->setText("Show");
        } else {
            editor->setText("Click");
        }

        connect(editor, SIGNAL(clicked()), signal_mapper, SLOT(map()));
        signal_mapper->setMapping(editor, index.row());

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
    int row = index.row();
    QString value = QString(FILEPATH.c_str()) + QString(dir_list.at(row).c_str());
    model->setData(index, value, Qt::EditRole);
}

void ButtonDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void ButtonDelegate::set_operation_mode(int mode)
{
    operation_mode = mode;
}

void ButtonDelegate::onButtonClicked(int row)
{
    if (operation_mode == 0) {
        QString filename = QFileDialog::getOpenFileName((QWidget*)_parent, tr("Open Image File 1"), "/home", tr("Image Files (*.png)"));
        if (filename == NULL || filename.size() == 0) {
            return;
        }
        QFileInfo file_info(filename);
        std::cout << ">>> filename: " << filename.toStdString() << std::endl;
        std::string dest_path = FILEPATH + dir_list.at(row) + std::string("/") + file_info.fileName().toStdString();
        std::cout << "ButtonDelegate::editorEvent::dest_path: " << dest_path << std::endl;
        if (QFile::copy(filename, QString(dest_path.c_str())) == false) {
            std::cout << "ButtonDelegate::editorEvent::ERROR: Can't copy file." << std::endl;
        }

        emit data_changed(row);
    } else if (operation_mode == 1) {
        //QString path = QDir::toNativeSeparators(QApplication::applicationDirPath());
        QString filepath = QString("file:///") + QString(FILEPATH.c_str()) + QString("/") + QString(dir_list.at(row).c_str());
        std::cout << ">>>>> filepath: " << filepath.toStdString() << std::endl;
        QDesktopServices::openUrl(QUrl(filepath));
    }
}


