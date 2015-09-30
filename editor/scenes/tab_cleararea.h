#ifndef TAB_CLEARAREA_H
#define TAB_CLEARAREA_H

#include <QWidget>

namespace Ui {
class TabClearArea;
}

class TabClearArea : public QWidget
{
    Q_OBJECT

public:
    explicit TabClearArea(QWidget *parent = 0);
    ~TabClearArea();

private:
    Ui::TabClearArea *ui;
};

#endif // TAB_CLEARAREA_H
