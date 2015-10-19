#ifndef ANIMTITLE_H
#define ANIMTITLE_H

#include <QWidget>
#include <QTimer>

class animTitle : public QWidget
{
    Q_OBJECT
public:
    explicit animTitle(QWidget *parent = 0);
    void update_properties();

protected:
  void paintEvent(QPaintEvent *event);

private:
  QWidget *myParent;
  int _sprite_n;
  int _timer_next_frame;
  QTimer *_timer;

  QImage image;
  int max_frames;

signals:

public slots:
    void updateBG();
};

#endif // ANIMTITLE_H
