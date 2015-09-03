#ifndef TEXT_H
#define TEXT_H

#include <QDialog>

namespace Ui {
class TabText;
}

class TabText : public QDialog
{
    Q_OBJECT

public:
    explicit TabText(QWidget *parent = 0);
    ~TabText();


private:
    void fill_list();

private:
    Ui::TabText *ui;
};

#endif // TEXT_H
