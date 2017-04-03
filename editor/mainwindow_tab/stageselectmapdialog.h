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

private:
    Ui::StageSelectMapDialog *ui;
};

#endif // STAGESELECTMAPDIALOG_H
