#ifndef STAGESELECTMAPEDITAREA_H
#define STAGESELECTMAPEDITAREA_H


#include <QDialog>

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class StageSelectMapEditArea : public QWidget
{
    Q_OBJECT
public:
    explicit StageSelectMapEditArea(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent * event);

signals:

public slots:

};


#endif // STAGESELECTMAPEDITAREA_H
