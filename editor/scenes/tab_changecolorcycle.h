#ifndef TAB_CHANGECOLORCYCLE_H
#define TAB_CHANGECOLORCYCLE_H

#include <QWidget>

namespace Ui {
class TabChangeColorcycle;
}

class TabChangeColorcycle : public QWidget
{
    Q_OBJECT

public:
    explicit TabChangeColorcycle(QWidget *parent = 0);
    ~TabChangeColorcycle();

private:
    Ui::TabChangeColorcycle *ui;
};

#endif // TAB_CHANGECOLORCYCLE_H
