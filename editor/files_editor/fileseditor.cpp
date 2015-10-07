#include "fileseditor.h"
#include "ui_fileseditor.h"
#include "model/combolistdelegate.h"

#include <QDir>
#include <map>

extern std::string FILEPATH;

FilesEditor::FilesEditor(QWidget *parent) : QMainWindow(parent), ui(new Ui::FilesEditor), model_directories(this)
{
    ui->setupUi(this);

    ui->dir_list_tableView->setModel(&model_directories);

    std::vector<std::string> dir_list = model_directories.get_directory_list();
    ComboListDelegate* delegate = new ComboListDelegate(this, get_dir_files_matrix(dir_list));
    ui->dir_list_tableView->setItemDelegateForColumn(1, delegate);
}

FilesEditor::~FilesEditor()
{
    delete ui;
}

std::map<int, std::vector<std::string>> FilesEditor::get_dir_files_matrix(std::vector<std::string> dir_list)
{
    std::map<int, std::vector<std::string>> res;
    for (int k=0; k<dir_list.size(); k++) {
        QString filepath = QString(FILEPATH.c_str()) + QString(dir_list.at(k).c_str());
        QDir dir = QDir(filepath);
        if (!dir.exists()) {
            std::cout << ">> FilesEditor::get_dir_files_matrix( ERROR: Directory '" << filepath.toStdString() << " does not exist. <<" << std::endl;
            exit(-1);
        }
        dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        dir.setSorting(QDir::Size | QDir::Reversed);
        QFileInfoList list = dir.entryInfoList();

        std::vector<std::string> file_list;
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fileInfo = list.at(i);
            if (fileInfo.fileName().length() > 50) {
                std::cout << "ERROR: file '" << fileInfo.fileName().toStdString() << "' surpasses the maximum number of file-characters (50)" << std::endl;
            } else {
                file_list.push_back(fileInfo.fileName().toStdString());
            }
        }
        res.insert(std::pair<int, std::vector<std::string>>(k, file_list));
    }
    return res;
}
