#include "passwordgrid.h"
#include "../../defines.h"
#include "../../scenes/password_generator.h"

#include <QPainter>

extern password_matrix matrix;

passwordGrid::passwordGrid(QWidget *parent) :
    QWidget(parent)
{
}

void passwordGrid::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QLineF line;
    int cell_number = PASSWORD_GRID_SIZE+1;
    int size = this->width() / cell_number;
    if (this->height() < this->width()) {
        size = this->height() / cell_number;
    }
    // horizontal lines
    for (int i=0; i<=cell_number; i++) {
        line = QLineF(0, i*size, cell_number*size, i*size);
        painter.drawLine(line);
    }
    // vertical lines
    for (int i=0; i<=cell_number; i++) {
        line = QLineF(i*size, 0, i*size, cell_number*size);
        painter.drawLine(line);
    }
    // draw horizontal labels
    for (int i=1; i<=PASSWORD_GRID_SIZE; i++) {
        painter.drawText(i*size, 0, size, size, Qt::AlignCenter, QString::number(i));
    }
    char letters[] = {'A', 'B', 'C', 'D', 'E', 'F'};
    // draw vertical labels
    for (int i=1; i<=PASSWORD_GRID_SIZE; i++) {
        painter.drawText(0, i*size, size, size, Qt::AlignCenter, QString(letters[i-1]));
    }
    for (int i=0; i<=PASSWORD_GRID_SIZE; i++) {
        for (int j=0; j<PASSWORD_GRID_SIZE; j++) {
            if (matrix.value[i][j] == PASSWORD_BALL_COLOR_BLUE) {
                painter.setBrush(QColor(0, 115, 239, 255));
                painter.drawEllipse((i+1)*size+2, (j+1)*size+2, size-4, size-4);
            } else if (matrix.value[i][j] == PASSWORD_BALL_COLOR_RED) {
                painter.setBrush(QColor(255, 51, 0, 255));
                painter.drawEllipse((i+1)*size+2, (j+1)*size+2, size-4, size-4);
            }
        }
    }
}
