#ifndef FILESEDITOR_H
#define FILESEDITOR_H

#include <QMainWindow>
#include "image_directories_model.h"

namespace Ui {
class FilesEditor;
}

class FilesEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit FilesEditor(QWidget *parent = 0);
    ~FilesEditor();
    std::map<int, std::vector<std::string> > get_dir_files_matrix(std::vector<std::string> dir_list);

public slots:
    void on_selected_image_changed(std::string);

private:
    Ui::FilesEditor *ui;
    ImageDirectoriesModel model_directories;
    std::map<int, std::vector<std::string> > data_matrix;
    std::vector<std::string> dir_list;
};

#endif // FILESEDITOR_H
