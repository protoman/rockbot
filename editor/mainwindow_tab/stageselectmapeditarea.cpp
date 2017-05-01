#include "stageselectmapeditarea.h"
#include "defines.h"
#include "mediator.h"

extern std::string FILEPATH;

StageSelectMapEditArea::StageSelectMapEditArea(QWidget *parent) : QWidget(parent)
{

}

void StageSelectMapEditArea::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    std::string filename = FILEPATH + "/images/backgrounds/stage_select_map.png";
    QPixmap bg_image(filename.c_str());

    if (bg_image.isNull() == false && bg_image.width() > 0) {

        QRectF pos_source(QPoint(0, 0), QSize(RES_W, RES_H));
        QRectF pos_dest(QPoint(0, 0), QSize(RES_W, RES_H));
        painter.drawPixmap(pos_dest, bg_image, pos_source);
    }

    // draw grid
    QPen pen(QColor(60, 60, 60), 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
    int pos;
    QLineF line;

    painter.setPen(pen);
    for (int i=0; i<(bg_image.width()/TILESIZE)+1; i++) {
        pos = i*TILESIZE;
        // linhas horizontais
        line = QLineF(pos, 0, pos, bg_image.height());
        painter.drawLine(line);
    }
    painter.setPen(pen);
    for (int i=0; i<(bg_image.height()/TILESIZE)+1; i++) {
        pos = i*TILESIZE;
        // linhas verticais
        line = QLineF(0, pos, bg_image.width(), pos);
        painter.drawLine(line);
    }

    // draw types - castle red, path blue, stage orange //
    for (int i=0; i<RES_W/TILESIZE; i++) {
        for (int j=0; j<RES_H/TILESIZE; j++) {
            short point_type = Mediator::get_instance()->stage_select_data.points[i][j];
            if (point_type == STAGE_SELECT_EDIT_MODE_PATH) {
                painter.setBrush(QColor(80, 255, 80, 180));
                painter.drawRect(i*TILESIZE, j*TILESIZE, TILESIZE, TILESIZE);
            } else if (point_type >= 10) {
                painter.setBrush(QColor(255, 102, 0, 180));
                painter.drawRect(i*TILESIZE, j*TILESIZE, TILESIZE, TILESIZE);
                painter.setPen(QColor(255, 255, 255, 255));
                painter.drawText(i*TILESIZE+TILESIZE/2, j*TILESIZE+TILESIZE/2, QString::number(point_type-10));
                painter.setPen(pen);
            } else if (point_type == STAGE_SELECT_EDIT_MODE_CASTLE) {
                painter.setBrush(QColor(255, 0, 0, 180));
                painter.drawRect(i*TILESIZE, j*TILESIZE, TILESIZE, TILESIZE);
            }

        }
    }

}

void StageSelectMapEditArea::mousePressEvent(QMouseEvent *event)
{
    QPoint pnt = event->pos();
    int editor_selectedTileX = pnt.x()/TILESIZE;
    int editor_selectedTileY = pnt.y()/TILESIZE;

    std::cout << "###  StageSelectMapEditArea::mousePressEvent.stage_select_edit_mode[" << (int)Mediator::get_instance()->stage_select_edit_mode << "]" << std::endl;

    // setting twice for the same point, returns it to locked
    if (Mediator::get_instance()->stage_select_data.points[editor_selectedTileX][editor_selectedTileY] == Mediator::get_instance()->stage_select_edit_mode) {
        Mediator::get_instance()->stage_select_data.points[editor_selectedTileX][editor_selectedTileY] = STAGE_SELECT_EDIT_MODE_LOCKED;
    } else {
        if (Mediator::get_instance()->stage_select_edit_mode == STAGE_SELECT_EDIT_MODE_STAGE) {
            Mediator::get_instance()->stage_select_data.points[editor_selectedTileX][editor_selectedTileY] = Mediator::get_instance()->stage_select_stage_number + 10;
        } else {
            Mediator::get_instance()->stage_select_data.points[editor_selectedTileX][editor_selectedTileY] = Mediator::get_instance()->stage_select_edit_mode;
        }
    }

    repaint();

}

