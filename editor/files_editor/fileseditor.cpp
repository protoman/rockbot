#include "fileseditor.h"
#include "ui_fileseditor.h"
#include "model/combolistdelegate.h"
#include "model/buttondelegate.h"

#include <QDir>
#include <map>

extern std::string FILEPATH;

FilesEditor::FilesEditor(QWidget *parent) : QMainWindow(parent), ui(new Ui::FilesEditor), model_directories(this)
{
    ui->setupUi(this);

    ui->dir_list_tableView->setModel(&model_directories);

    dir_list = model_directories.get_directory_list();
    data_matrix = get_dir_files_matrix(dir_list);

    //ComboListDelegate* delegate = new ComboListDelegate(this, data_matrix);
    ButtonDelegate* delegate_button_preview = new ButtonDelegate(this, dir_list, data_matrix, &model_directories);
    delegate_button_preview->set_operation_mode(1);
    ui->dir_list_tableView->setItemDelegateForColumn(1, delegate_button_preview);

    ButtonDelegate* delegate_button = new ButtonDelegate(this, dir_list, data_matrix, &model_directories);
    ui->dir_list_tableView->setItemDelegateForColumn(2, delegate_button);


    for ( int i = 0; i < model_directories.rowCount(); ++i ) {
        ui->dir_list_tableView->openPersistentEditor( model_directories.index(i, 1) );
        ui->dir_list_tableView->openPersistentEditor( model_directories.index(i, 2) );
    }

    connect(&model_directories, SIGNAL(selected_image_changed(std::string)), this, SLOT(on_selected_image_changed(std::string)));


    connect(delegate_button, SIGNAL(data_changed(int)), this, SLOT(on_data_changed(int)));

    ui->image_preview_widget->set_ignore_lines(true);


}

FilesEditor::~FilesEditor()
{
    delete ui;
}

std::map<int, std::vector<std::string> > FilesEditor::get_dir_files_matrix(std::vector<std::string> dir_list)
{
    std::map<int, std::vector<std::string> > res;
    for (int k=0; k<dir_list.size(); k++) {
        QString filepath = QString(FILEPATH.c_str()) + QString(dir_list.at(k).c_str());
        QDir dir = QDir(filepath);
        if (!dir.exists()) {
            exit(-1);
        }
        dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        dir.setSorting(QDir::Size | QDir::Reversed);
        QFileInfoList list = dir.entryInfoList();

        std::vector<std::string> file_list;
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fileInfo = list.at(i);
            if (fileInfo.fileName().length() <= 50) {
                file_list.push_back(fileInfo.fileName().toStdString());
            }
        }
        res.insert(std::pair<int, std::vector<std::string> >(k, file_list));
    }
    return res;
}

void FilesEditor::on_selected_image_changed(std::string image)
{
    QString filename = QString(FILEPATH.c_str()) + QString("/") + QString(image.c_str());
    ui->image_preview_widget->setImageFilename(filename);
}

