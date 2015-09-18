#ifndef SCENEPREVIEWAREA_H
#define SCENEPREVIEWAREA_H

#include <QWidget>
#include <QTimer>

#include "scenes/scenesmediator.h"

#define TIMER_DELAY 10

class ScenePreviewArea : public QWidget
{
    Q_OBJECT
public:
    explicit ScenePreviewArea(QWidget *parent = 0);
    void change_play_state();
    void reset_move_vars();
    void init_image_scene_move();
    void show_error(QString error_msg);

signals:
    void play_state_changed(bool state);

public slots:
    void timer_check();

protected:
    void paintEvent(QPaintEvent *event);


private:
    QTimer *_timer;
    qint64 _next_timer;                                    // set this to the timestamp you want to timer_check to execute
    bool play_state;
    ScenesMediator *mediator;
    int scene_n;                                        // control witch scene you are playing
    int image_n;                                        // control witch image you are playing
    bool playing_image;
    int image_frame;                                    // control witch image animation frame you are in
    QString graphic_filename;
    float speed_x;
    float speed_y;
    float inc_x;
    float inc_y;
    int current_scene_image_n;
};

#endif // SCENEPREVIEWAREA_H
