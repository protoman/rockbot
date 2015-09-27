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
    void set_selected_n(int n);

protected:
    void paintEvent(QPaintEvent *event);


private:
    int selected_n;
    QFont monospace;
};

#endif // TEXTPREVIEWAREA_H
