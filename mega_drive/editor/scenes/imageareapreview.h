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

    void set_ini_x(int set);
    void set_ini_y(int set);
    void set_dest_x(int set);
    void set_dest_y(int set);
    void set_ignore_lines(bool set_ignore);

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
    int ini_x;
    int ini_y;
    int dest_x;
    int dest_y;
    bool ignore_lines;
};

#endif // IMAGEAREAPREVIEW_H
