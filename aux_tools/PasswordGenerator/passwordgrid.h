#ifndef PASSWORDGRID_H
#define PASSWORDGRID_H

#include <QWidget>

class passwordGrid : public QWidget
{
    Q_OBJECT
public:
    explicit passwordGrid(QWidget *parent = 0);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event);

};

#endif // PASSWORDGRID_H
