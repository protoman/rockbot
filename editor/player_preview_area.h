#ifndef PLAYER_PREVIEW_AREA_H
#define PLAYER_PREVIEW_AREA_H

#include <QWidget>
#include <QImage>
#include <QPixmap>

class player_preview_area : public QWidget
{
    Q_OBJECT
public:
    explicit player_preview_area(QWidget *parent = 0);


signals:

public slots:
    void update_sprites();






public:
	void change_player(int player_n);

protected:
  void paintEvent(QPaintEvent *event);

private:
    void replace_colors();

private:
	int selected_player;
	QWidget *myParent;
    QImage _colored_sprites;
    QImage _original_sprites;
};

#endif // PLAYER_PREVIEW_AREA_H
