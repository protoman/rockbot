#ifndef TAB_VIEWPOINT_H
#define TAB_VIEWPOINT_H

#include <QWidget>

namespace Ui {
class tab_viewpoint;
}

class tab_viewpoint : public QWidget
{
    Q_OBJECT

public:
    explicit tab_viewpoint(QWidget *parent = 0);
    ~tab_viewpoint();

private:
    Ui::tab_viewpoint *ui;
};

#endif // TAB_VIEWPOINT_H
