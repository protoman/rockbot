#include "animation_previewarea.h"

extern std::string FILEPATH;

AnimationPreviewArea::AnimationPreviewArea(QWidget *parent) : QWidget(parent)
{
    animation_timer = 0;
    frame_n = 0;
    x = 0;
    y = 0;
    w = 0;
    h = 0;
    delay = 10;

    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(update_animation()));
    _timer->start(10);

}

AnimationPreviewArea::~AnimationPreviewArea()
{
    _timer->stop();
}

void AnimationPreviewArea::set_filename(QString name)
{
    graphic_filename = QString(FILEPATH.c_str()) + QString("/images/scenes/animations/") + name;
}

void AnimationPreviewArea::set_w(int set)
{
    w = set;
}

void AnimationPreviewArea::set_h(int set)
{
    h = set;
}

void AnimationPreviewArea::set_delay(int set)
{
    delay = set;
}

void AnimationPreviewArea::set_x(int set)
{
    x = set;
}

void AnimationPreviewArea::set_y(int set)
{
    y = set;
}


void AnimationPreviewArea::update_animation()
{
    //std::cout << ">> AnimationPreviewArea::update_animation - frame_n: " << frame_n << std::endl;
    unsigned int now_time = QDateTime::currentMSecsSinceEpoch();
    if (now_time > animation_timer) {
        //std::cout << ">> AnimationPreviewArea::update_animation - RUN!" << std::endl;
        animation_timer = now_time + delay;
        frame_n++;
        repaint();
    }
}

void AnimationPreviewArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRectF target, source;

    painter.fillRect(0, 0, RES_W, RES_H, QColor(0, 0, 0, 255));

    if (graphic_filename.length() == 0) {
        std::cout << ">> ImageAreaPreview::paintEvent: LEAVE #1" << std::endl;
        return;
    }
    if (graphic_filename.toStdString().find(".png") == std::string::npos) {
        std::cout << ">> ImageAreaPreview::paintEvent: LEAVE #3" << std::endl;
        return;
    }

    QPixmap image(graphic_filename.toStdString().c_str());

    if (image.isNull() == true || image.width() <= 0) {
        std::cout << ">> ImageAreaPreview::paintEvent: LEAVE #4" << std::endl;
        return;
    }

    if (w == 0) {
        // @TODO: isto deve gerar um SIGNAL que a UI vai receber para atualizar o valor na spinbox
        w = image.width();
        emit on_image_w_changed(w);
    }
    if (h == 0) {
        // @TODO: isto deve gerar um SIGNAL que a UI vai receber para atualizar o valor na spinbox
        h = image.height();
        emit on_image_h_changed(h);
    }

    // reset animation when needed
    //std::cout << "frame_n: " << frame_n << ", frame_n*w: " << (frame_n*w) << ", image.width(): " << image.width() << std::endl;
    if (frame_n*w >= image.width()) {
        frame_n = 0;
    }

    target = QRectF(QPoint(x, y), QSize(w, h));
    source = QRectF(QPoint(frame_n*w, 0), QSize(w, h));
    painter.drawPixmap(target, image, source);

}

