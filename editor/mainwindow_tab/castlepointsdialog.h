#ifndef CASTLEPOINTSDIALOG_H
#define CASTLEPOINTSDIALOG_H

#include <QDialog>

namespace Ui {
class CastlePointsDialog;
}

class CastlePointsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CastlePointsDialog(QWidget *parent = nullptr);
    ~CastlePointsDialog();

private:
    Ui::CastlePointsDialog *ui;
};

#endif // CASTLEPOINTSDIALOG_H
