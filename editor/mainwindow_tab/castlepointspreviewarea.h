#ifndef CASTLEPOINTSPREVIEWAREA_H
#define CASTLEPOINTSPREVIEWAREA_H

#include <QWidget>
#include <QPainter>

class CastlePointsPreviewArea : public QWidget
{
    Q_OBJECT
public:
    explicit CastlePointsPreviewArea(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);

signals:

public slots:
};

#endif // CASTLEPOINTSPREVIEWAREA_H
