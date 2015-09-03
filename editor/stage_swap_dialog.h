#ifndef STAGE_SWAP_DIALOG_H
#define STAGE_SWAP_DIALOG_H

#include <QDialog>

namespace Ui {
class stage_swap_dialog;
}

class stage_swap_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit stage_swap_dialog(QWidget *parent = 0);
    ~stage_swap_dialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::stage_swap_dialog *ui;
};

#endif // STAGE_SWAP_DIALOG_H
