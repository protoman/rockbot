#ifndef PARALLAXPREVIEWAREA_H
#define PARALLAXPREVIEWAREA_H

#include "defines.h"
#include "file/v4/file_scene_v4.h"
#include <string>

#include <QWidget>

class parallaxpreviewarea : public QWidget
{
    Q_OBJECT
public:
    explicit parallaxpreviewarea(QWidget *parent = nullptr);
    void set_filename(int layer_n, std::string new_filename);
    void set_y(int layer_n, int y);
    void set_h(int layer_n, int h);

signals:

protected:
    void paintEvent(QPaintEvent *event);

private:
    std::string filename[PARALLAX_LAYERS_MAX];
    int ypos[PARALLAX_LAYERS_MAX];
    int hpos[PARALLAX_LAYERS_MAX];
};

#endif // PARALLAXPREVIEWAREA_H
