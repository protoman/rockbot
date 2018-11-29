#ifndef CASTLEPOINTSDIALOG_H
#define CASTLEPOINTSDIALOG_H

#include <QDialog>

#include "common.h"
#include "mediator.h"

namespace Ui {
class CastlePointsDialog;
}

class CastlePointsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CastlePointsDialog(QWidget *parent = nullptr);
    ~CastlePointsDialog();
    void load_data();

private slots:
    void on_point_comboBox_currentIndexChanged(int index);

    void on_x_spinBox_valueChanged(int arg1);

    void on_y_spinBox_valueChanged(int arg1);

private:
    Ui::CastlePointsDialog *ui;
    int currentPoint;
    bool data_loading;

};

#endif // CASTLEPOINTSDIALOG_H
