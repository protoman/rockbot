#ifndef STAGESELECTMAPDIALOG_H
#define STAGESELECTMAPDIALOG_H

#include <QDialog>

namespace Ui {
class StageSelectMapDialog;
}

class StageSelectMapDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StageSelectMapDialog(QWidget *parent = 0);
    ~StageSelectMapDialog();

private slots:
    void on_path_pushButton_clicked();

    void on_stage_pushButton_clicked();

    void on_castle_pushButton_clicked();

    void on_stageNumber_spinBox_valueChanged(int arg1);

private:
    Ui::StageSelectMapDialog *ui;
};

#endif // STAGESELECTMAPDIALOG_H
