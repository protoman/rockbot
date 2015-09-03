#ifndef DIALOG_PICK_COLOR_H
#define DIALOG_PICK_COLOR_H

#include <QDialog>
#include <QSignalMapper>
#include "../file/format.h"


namespace Ui {
    class dialog_pick_color;
}

class dialog_pick_color : public QDialog
{
    Q_OBJECT

public slots:
	void clicked(const int &color_n);

signals:
	void accepted();


public:
    explicit dialog_pick_color(QWidget *parent = 0);
    ~dialog_pick_color();

private slots:
	void on_buttonBox_accepted();

private:
    Ui::dialog_pick_color *ui;
	QSignalMapper *signalMapper;
};

#endif // DIALOG_PICK_COLOR_H
