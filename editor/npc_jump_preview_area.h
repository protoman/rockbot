#ifndef NPC_JUMP_PREVIEW_AREA_H
#define NPC_JUMP_PREVIEW_AREA_H

#include <QWidget>
#include <QPainter>
#include <QBitmap>
#include <QTimer>
#include <aux_tools/trajectory_parabola.h>
#include "mediator.h"

class npc_jump_preview_area : public QWidget
{
    Q_OBJECT
public:
    explicit npc_jump_preview_area(QWidget *parent = nullptr);
    void set_graphicfile(std::string filename);
    std::string get_graphicfile();

protected:
    void paintEvent(QPaintEvent *event);
    QWidget *myParent;

private:
    void reset_pos();
    QTimer *timer;
    int calc_jump_pos_x(int distance);
    std::string graphic_filename;
    trajectory_parabola *parabola_arc;          // used for jumping to a specific point
    int xinx_multiplier = 1;
    int move_speed = 1;
    st_float_position position = st_float_position(0, 0);

public slots:
    void updateBG();

signals:
};

#endif // NPC_JUMP_PREVIEW_AREA_H
