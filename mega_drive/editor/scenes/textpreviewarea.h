#ifndef TEXTPREVIEWAREA_H
#define TEXTPREVIEWAREA_H

#include "defines.h"
#include "scenes/scenesmediator.h"
#include <QWidget>
#include <QFont>

class TextPreviewArea : public QWidget
{
public:
    explicit TextPreviewArea(QWidget *parent = 0);
    void set_selected_n(int n, std::string text_list[]);

protected:
    void paintEvent(QPaintEvent *event);


private:
    int selected_n;
    QFont monospace;
    std::string scene_text_list[SCENE_TEXT_LINES_N];
};

#endif // TEXTPREVIEWAREA_H
