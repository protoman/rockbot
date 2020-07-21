#ifndef TAB_PARALLAX_H
#define TAB_PARALLAX_H

#include <QWidget>

namespace Ui {
class tab_parallax;
}

class tab_parallax : public QWidget
{
    Q_OBJECT

public:
    explicit tab_parallax(QWidget *parent = nullptr);
    ~tab_parallax();

private:
    Ui::tab_parallax *ui;
};

#endif // TAB_PARALLAX_H
