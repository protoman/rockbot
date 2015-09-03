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
extern Mediator *dataExchanger;


class EditorTilePallete : public QWidget
{
  Q_OBJECT

public:
    // methods
    EditorTilePallete(QWidget *parent = 0);
    int selectedTileX, selectedTileY;
    QString getPallete();
    QString currentPallete;

  // variables
  QWidget *myParent;

protected:
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent * event);

signals:
     void signalPalleteChanged();

public slots:
   void changeTileSet(const QString &tileset);
};
#endif
