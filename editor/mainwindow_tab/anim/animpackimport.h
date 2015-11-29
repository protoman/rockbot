#ifndef ANIMPACKIMPORT_H
#define ANIMPACKIMPORT_H

#include <QDialog>
#include <vector>

namespace Ui {
class AnimPackImport;
}

class AnimPackImport : public QDialog
{
    Q_OBJECT

public:
    explicit AnimPackImport(QWidget *parent = 0);
    ~AnimPackImport();

private:
    void fill_frames_delay_combo(int n);

private slots:

    void on_imageFilename_toolButton_clicked();

    void on_frameDelay_spinBox_valueChanged(int arg1);

    void on_buttonBox_accepted();

private:
    Ui::AnimPackImport *ui;
    bool data_loading;
    std::vector<int> delay_list;
    QString filename;
    QPixmap image;
};

#endif // ANIMPACKIMPORT_H
