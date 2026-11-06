#ifndef NEWGAMEDIALOG_H
#define NEWGAMEDIALOG_H

#include <QDialog>

namespace Ui {
class NewGameDialog;
}

class NewGameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewGameDialog(QWidget *parent = 0);
    ~NewGameDialog();

private:
    bool game_folder_exists(QString);

private slots:
    void on_gameName_lineEdit_textChanged(const QString &arg1);

    void on_buttonBox_accepted();

signals:
    void on_accepted(QString);

private:
    Ui::NewGameDialog *ui;
    QString game_name;
    bool data_loading;
};

#endif // NEWGAMEDIALOG_H
