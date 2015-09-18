#include <QPainter>
#include <QDateTime>
#include <QMessageBox>
#include <iostream>

#include "scenepreviewarea.h"

extern char EDITOR_FILEPATH[512];

ScenePreviewArea::ScenePreviewArea(QWidget *parent) : QWidget(parent)
{
    play_state = false;
    mediator = ScenesMediator::get_instance();
    scene_n = 0;
    image_n = 0;
    playing_image = false;
    image_frame = 0;

    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(timer_check()));
    current_scene_image_n = 0;
    reset_move_vars();
}

void ScenePreviewArea::change_play_state()
{
    play_state = !play_state;
    playing_image = play_state;
    scene_n = 0;
    image_n = 0;
    image_frame = 0;


    if (play_state == true) {
        emit play_state_changed(true);
        reset_move_vars();
        if (_timer->isActive() == true) {
            show_error("ERROR: Timer is still active while trying to play again!");
            return;
        }
        _timer->start(TIMER_DELAY);
        // play image
        if (mediator->image_list.size() > 0) {
            current_scene_image_n = 0;
            init_image_scene_move();
        }
    } else {
        _timer->stop();
        emit play_state_changed(false);
    }


}

void ScenePreviewArea::reset_move_vars()
{
    speed_x = 1;
    speed_y = 1;
    inc_x = 0;
    inc_y = 0;
}

void ScenePreviewArea::init_image_scene_move()
{
    speed_x = 1;
    speed_y = 1;
    inc_x = 0;
    inc_y = 0;
    CURRENT_FILE_FORMAT::file_scene_show_image current_scene_image = mediator->image_list.at(current_scene_image_n);
    int diff_x = abs(current_scene_image.ini_x - current_scene_image.dest_x);
    int diff_y = abs(current_scene_image.ini_y - current_scene_image.dest_y);
    if (diff_x > diff_y) {
        speed_y = (float)((float)diff_y / diff_x);
        std::cout << "speed_y: " << speed_y << ", res: " << ((float)diff_y / diff_x) << std::endl;
    } else if (diff_x < diff_y) {
        speed_x = (float)((float)diff_y / diff_x);
    }
    if (current_scene_image.ini_x > current_scene_image.dest_x) {
        speed_x = -speed_x;
    }
    if (current_scene_image.ini_y > current_scene_image.dest_y) {
        speed_y = -speed_y;
    }
    _next_timer = QDateTime::currentMSecsSinceEpoch() + current_scene_image.move_delay;
    std::cout << "now_timer: " << QDateTime::currentMSecsSinceEpoch() << ", _next_timer: " << _next_timer << std::endl;
    graphic_filename = QString(EDITOR_FILEPATH) + QString("/data/images/scenes/") + mediator->image_list.at(0).filename;
    repaint();
}

void ScenePreviewArea::show_error(QString error_msg)
{
    QMessageBox msgBox;
    msgBox.setText(error_msg);
    msgBox.exec();
}

void ScenePreviewArea::timer_check()
{
    if (play_state == false) {
        return;
    }
    // time enough has passed, execute action
    if (_next_timer <= QDateTime::currentMSecsSinceEpoch()) {
        // @TODO - check if there is a loop and reverse flags
        if (inc_x == mediator->image_list.at(current_scene_image_n).dest_x) {
            // @TODO - pass to next scene
            _timer->stop();
            play_state = false;
            emit play_state_changed(false);
            return;
        }
        std::cout << "ScenePreviewArea::timer_check - RUN #1, image_frame: " << image_frame << ", speed_y: " << speed_y << std::endl;
        inc_x += speed_x;
        inc_y += speed_y;
        repaint();
    }
}

void ScenePreviewArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRectF target, source;

    painter.fillRect(0, 0, this->width(), this->height(), QColor(0, 0, 0, 255));

    if (play_state == false) {
        return;
    }

    if (playing_image == true) {

        if (graphic_filename.toStdString().find(".png") == std::string::npos) {
            std::cout << ">> ScenePreviewArea::paintEvent: LEAVE #3" << std::endl;
            return;
        }

        QPixmap image(graphic_filename.toStdString().c_str());

        if (image.isNull() == true || image.width() <= 0) {
            std::cout << ">> ScenePreviewArea::paintEvent: LEAVE #4" << std::endl;
            return;
        }

        std::cout << "ScenePreviewArea::paintEvent - image_frame: " << image_frame << std::endl;
        target = QRectF(QPoint(mediator->image_list.at(0).ini_x+inc_x, mediator->image_list.at(0).ini_y+inc_y), QSize(image.width(), image.height()));
        source = QRectF(QPoint(0, 0), QSize(image.width(), image.height()));
        painter.drawPixmap(target, image, source);
    }

}

