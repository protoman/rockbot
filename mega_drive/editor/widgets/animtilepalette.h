#ifndef ANIMTILEPALETTE_H
#define ANIMTILEPALETTE_H

#include <QWidget>
#include <QPixmap>

class animTilePalette : public QWidget
{
    Q_OBJECT
public:
    explicit animTilePalette(QWidget *parent = 0);
    int selectedTileX, selectedTileY;
    QString getPallete();
    void reload();


protected:
  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent * event);

signals:
     void signalPalleteChanged();

public slots:
   void changeTileSet(const QString &tileset);

private:
  QWidget *myParent;
    std::vector<QPixmap> image_list;

};

#endif // ANIMTILEPALETTE_H
