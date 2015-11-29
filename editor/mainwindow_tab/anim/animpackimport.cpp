#include "animpackimport.h"
#include "ui_animpackimport.h"
#include "common.h"

#include <QString>
#include <QFileDialog>
#include <QPainter>

extern std::string FILEPATH;

AnimPackImport::AnimPackImport(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AnimPackImport)
{
    filename = QString("");
    ui->setupUi(this);
}

AnimPackImport::~AnimPackImport()
{
    delete ui;
}

void AnimPackImport::fill_frames_delay_combo(int n)
{
    ui->frameN_comboBox->clear();
    delay_list.clear();

    for (int i=0; i<n; i++) {
        QString name = QString("[") + QString::number(i) + QString("]");
        ui->frameN_comboBox->addItem(name);
        delay_list.push_back(100);
    }
}


void AnimPackImport::on_imageFilename_toolButton_clicked()
{
    filename = QFileDialog::getOpenFileName((QWidget*)this, tr("Open Image File"), "/home", tr("Image Files (*.png)"));
    if (filename == NULL || filename.size() == 0) {
        return;
    }
    ui->imageFilename_lineEdit->setText(filename);
    image = QPixmap(filename);
    if (image.isNull()) {
        return;
    }
    int n = image.height()/TILESIZE;
    fill_frames_delay_combo(n);

}

void AnimPackImport::on_frameDelay_spinBox_valueChanged(int arg1)
{
    if (delay_list.size() <= ui->frameN_comboBox->currentIndex()) {
        std::cout << "ERROR: delay vector size is invalid." << std::endl;
        return;
    }
    delay_list.at(ui->frameN_comboBox->currentIndex()) = arg1;
}

void AnimPackImport::on_buttonBox_accepted()
{
    if (filename.length() == 0) {
        return;
    }
    if (image.isNull()) {
        return;
    }

    int cols = image.width()/TILESIZE;
    int rows = image.height()/TILESIZE;
    for (int i=0; i<cols; i++) {
        QPixmap new_image(TILESIZE*rows, TILESIZE);
        QPainter painter(&new_image);
        for (int j=0; j<rows; j++) {
            QPixmap new_tile = image.copy(i*TILESIZE, j*TILESIZE, TILESIZE, TILESIZE);
            painter.drawPixmap(j*TILESIZE, 0, new_tile);
        }
        QString new_filename = QString(FILEPATH.c_str()) + QString("/images/tilesets/anim/TEST_") + QString::number(i) + QString(".png");
        QFile new_file(new_filename);
        new_file.open(QIODevice::WriteOnly);
        new_image.save(&new_file, "PNG");
    }

    /*
    // save pixmap to PNG file //
    QFile file("yourFile.png");
    file.open(QIODevice::WriteOnly);
    pixmap.save(&file, "PNG");
    */

}
