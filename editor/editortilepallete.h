#ifndef EDITORTILEPALLETE
#define EDITORTILEPALLETE

#include <QWidget>
#include <QShortcut>
#include <QPainter>
#include <QTimer>
#include <QScrollArea>
#include <QMessageBox>
#include <QColor>
#include <QMouseEvent>
#include "mediator.h"

extern int palleteX;
extern int palleteY;

class EditorTilePallete : public QWidget
{
  Q_OBJECT

public:
    // methods
    EditorTilePallete(QWidget *parent = 0);
    int selectedTileX, selectedTileY;
    QString getPallete();

  // variables
  QWidget *myParent;

protected:
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent * event);
  void wheelEvent(QWheelEvent *event);


private:
  int tilesize = 32;
  int zoom = 2;

signals:
     void signalPalleteChanged();

public slots:
   void changeTileSet(const QString &tileset);
};
#endif
