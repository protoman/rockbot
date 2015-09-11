#include <QPainter>
#include <QDateTime>
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
    _timer->start(TIMER_DELAY);
}

void ScenePreviewArea::change_play_state()
{
    play_state = !play_state;
    playing_image = play_state;
    scene_n = 0;
    image_n = 0;
    image_frame = 0;


    if (play_state == true) {
        // play image
        if (mediator->image_list.size() > 0) {
            _next_timer = QDateTime::currentMSecsSinceEpoch() + mediator->image_list.at(0).move_delay;
            std::cout << "now_timer: " << QDateTime::currentMSecsSinceEpoch() << ", _next_timer: " << _next_timer << std::endl;
            graphic_filename = QString(EDITOR_FILEPATH) + QString("/data/images/scenes/") + mediator->image_list.at(0).filename;
            repaint();
        }
    }


}

void ScenePreviewArea::timer_check()
{
    if (play_state == false) {
        return;
    }
    // time enough has passed, execute action
    if (_next_timer <= QDateTime::currentMSecsSinceEpoch()) {
        std::cout << "ScenePreviewArea::timer_check - RUN #1, image_frame: " << image_frame << std::endl;
        image_frame++;
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
        target = QRectF(QPoint(mediator->image_list.at(0).ini_x+image_frame, mediator->image_list.at(0).ini_y), QSize(image.width(), image.height()));
        source = QRectF(QPoint(0, 0), QSize(image.width(), image.height()));
        painter.drawPixmap(target, image, source);
    }

}

