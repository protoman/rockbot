#ifndef PLAYER_PREVIEW_AREA_H
#define PLAYER_PREVIEW_AREA_H

#include <QWidget>

class player_preview_area : public QWidget
{
    Q_OBJECT
public:
    explicit player_preview_area(QWidget *parent = 0);


signals:

public slots:






public:
	void change_player(int player_n);

protected:
  void paintEvent(QPaintEvent *event);

private:
  int get_colorn_from_rgb(QImage &image, int r, int g, int b);


private:
	int selected_player;
	QWidget *myParent;
};

#endif // PLAYER_PREVIEW_AREA_H
