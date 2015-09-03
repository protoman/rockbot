#ifndef LOADGAMEPICKER_H
#define LOADGAMEPICKER_H

#include <QDialog>

namespace Ui {
    class loadGamePicker;
}

class loadGamePicker : public QDialog {
    Q_OBJECT
public:
    loadGamePicker(QWidget *parent = 0);
    ~loadGamePicker();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::loadGamePicker *ui;

private slots:
	void on_buttonBox_accepted();
};

#endif // LOADGAMEPICKER_H
