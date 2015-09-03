#ifndef COLORCYCLE_MAP_PREVIEW_H
#define COLORCYCLE_MAP_PREVIEW_H

#include <QWidget>
#include "mediator.h"

#ifndef EDITORTILEPALLETE
	#include "editortilepallete.h"
#endif

#ifndef FILE_GAME
	#include "../file/format.h"
#endif

class colorcycle_map_preview : public QWidget
{
	Q_OBJECT
public:
	explicit colorcycle_map_preview(QWidget *parent = 0);


private:
	int get_colorn_from_rgb(QImage &image, int r, int g, int b);
    bool color1_next(unsigned int now_time);
    bool color2_next(unsigned int now_time);
    bool color3_next(unsigned int now_time);
    void apply_colorcycle(QImage&image);


private:
    QWidget *myParent;
    int _current_color1;
    int _current_color2;
    int _current_color3;
    QTimer *_timer;
    short _zoom;
    unsigned int _timer1;
    unsigned int _timer2;
    unsigned int _timer3;


protected:
	void paintEvent(QPaintEvent *);

signals:
	
public slots:
    void timer_check();

};

#endif // COLORCYCLE_MAP_PREVIEW_H
