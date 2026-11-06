#ifndef ANIMATIONPREVIEWAREA_H
#define ANIMATIONPREVIEWAREA_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QDateTime>
#include <iostream>
#include "defines.h"

class AnimationPreviewArea : public QWidget
{
    Q_OBJECT
public:
    explicit AnimationPreviewArea(QWidget *parent = 0);
    ~AnimationPreviewArea();
    void set_filename(QString name);
    void set_w(int set);
    void set_h(int set);
    void set_delay(int set);
    void set_x(int set);
    void set_y(int set);


protected:
    void paintEvent(QPaintEvent *event);

signals:
    void on_image_w_changed(int value);
    void on_image_h_changed(int value);

public slots:
    void update_animation();

private:
    QString graphic_filename;
    QTimer *_timer;
    int w;
    int h;
    int x;
    int y;
    int delay;
    int animation_timer;
    int frame_n;
};

#endif // ANIMATIONPREVIEWAREA_H
