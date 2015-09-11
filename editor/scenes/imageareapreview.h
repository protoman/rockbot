#ifndef IMAGEAREAPREVIEW_H
#define IMAGEAREAPREVIEW_H

#include <QWidget>
#include <QString>

class ImageAreaPreview : public QWidget
{
    Q_OBJECT
public:
    explicit ImageAreaPreview(QWidget *parent = 0);
    void setImageFilename(QString name);
    void setImageArea(int set_x, int set_y, int set_w, int set_h);
    void set_x(int set);
    void set_y(int set);
    void set_w(int set);
    void set_h(int set);

signals:
    void on_image_w_changed(int value);
    void on_image_h_changed(int value);

public slots:

protected:
    void paintEvent(QPaintEvent *event);


private:
    QString graphic_filename;
    int x;
    int y;
    int w;
    int h;
};

#endif // IMAGEAREAPREVIEW_H
