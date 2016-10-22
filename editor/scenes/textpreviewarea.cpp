#include "textpreviewarea.h"
#include <QPainter>
#include <iostream>
#include <QPaintEvent>
#include <QString>
#include <QFontDatabase>

#include "mediator.h"
#include "file/fio_strings.h"

extern std::string FILEPATH;
extern std::string GAMEPATH;

TextPreviewArea::TextPreviewArea(QWidget *parent) : QWidget(parent)
{
    selected_n = 0;
    const QString font_filename = QString(GAMEPATH.c_str()) + QString("/fonts/pressstart2p.ttf");
    int id = QFontDatabase::addApplicationFont(font_filename);
    QString font_family = QFontDatabase::applicationFontFamilies(id).at(0);
    monospace = QFont(font_family);
    monospace.setPixelSize(FONT_SIZE);
}

void TextPreviewArea::set_selected_n(int n)
{
    selected_n = n;
}

void TextPreviewArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setFont(monospace);

    painter.setPen(QColor(0, 0, 0));
    painter.fillRect(QRectF(0.0, 0.0, RES_W, RES_H), QColor(0, 0, 0, 255));

    if (ScenesMediator::get_instance()->text_list.size() < 1 || ScenesMediator::get_instance()->text_list.size() < selected_n) {
        //std::cout << ">> DON't DRAW TEXT PREVIEW << " << std::endl;
        return;
    }

    //std::cout << ">>OK - DRAW TEXT PREVIEW << " << std::endl;
    painter.setPen(QColor(255, 255, 255, 255));

    CURRENT_FILE_FORMAT::file_scene_show_text text_info = ScenesMediator::get_instance()->text_list.at(selected_n);

    int pos_x = 0;
    int pos_y = 0;
    /*
    text_position_type_dialogbottom,
    text_position_type_dialogtop,
    text_position_type_centered,
    text_position_type_center_x,
    text_position_type_center_y,
    text_position_type_user_defined
    */

    int lines_n = 0;
    int max_line_w = 0;
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    for (int i=0; i<SCENE_TEXT_LINES_N; i++) {
        QString line = QString(fio_str.get_scenes_string(text_info.line_string_id[i]).c_str());
        if (line.size() > 0) {
            if (line.size() > max_line_w) {
                max_line_w = line.size();
            }
            lines_n = i+1;
        }
    }

    int center_x = (RES_W * 0.5) - (max_line_w/2 * FONT_SIZE);
    int center_y = (RES_H * 0.5) - (lines_n * (SCENES_LINE_H_DIFF * 0.5));


    if (text_info.position_type == CURRENT_FILE_FORMAT::text_position_type_dialogbottom) {
        pos_x = 10;
        pos_y = SCENES_TEXT_BOTTOM_POSY;
    } else if (text_info.position_type == CURRENT_FILE_FORMAT::text_position_type_dialogtop) {
        pos_x = 10;
        pos_y = 10;
    } else if (text_info.position_type == CURRENT_FILE_FORMAT::text_position_type_centered) {
        pos_x = center_x;
        pos_y = center_y;

    } else if (text_info.position_type == CURRENT_FILE_FORMAT::text_position_type_center_x) {
        pos_x = center_x;
        pos_y = text_info.y;

    } else if (text_info.position_type == CURRENT_FILE_FORMAT::text_position_type_center_y) {
        pos_x = text_info.x;
        pos_y = center_y;

    } else if (text_info.position_type == CURRENT_FILE_FORMAT::text_position_type_user_defined) {
        pos_x = text_info.x;
        pos_y = text_info.y;
    }

    // don't know why Qt needs adding that, maybe because it counts from font bottom instead of top
    pos_y += FONT_SIZE;

    for (int i=0; i<lines_n; i++) {
        QString line = QString(fio_str.get_scenes_string(text_info.line_string_id[i]).c_str());
        painter.drawText(pos_x, pos_y + (i*SCENES_LINE_H_DIFF), line);
        //std::cout << ">>OK - DRAW LINE[" << i << "]" << std::endl;
    }

}
