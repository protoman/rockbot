#include <iostream>
#include <stdio.h>
#include "editorarea.h"
#include <QApplication>
#include <QResource>
#include <QBrush>
#include <QDir>


#include "defines.h"


#include "../file/format.h"
#include "../file/file_io.h"

extern std::string FILEPATH;

// construtor
EditorArea::EditorArea(QWidget *parent) : QWidget(parent) {
	mouse_released = true;
    myParent = parent;
    temp = 0;
    editor_selectedTileX = 0;
    editor_selectedTileY = 0;
	tempX = -1;
	tempY = -1;
    editor_selected_object_pos = 0;
    editor_selected_object_pos_map = 0;
    selection_started = false;
    selection_start_x = 0;
    selection_start_y = 0;
    selection_current_x = 0;
    selection_current_y = 0;

    update_files();

    this->show();
}

void EditorArea::changeTile() {
    update_files();
    temp = 1;
    repaint();
}


void EditorArea::update_files()
{

    std::string filename_str = FILEPATH + "images/tilesets/blocks/easymode.png";
    easy_mode_tile = QPixmap(QString(filename_str.c_str()));

    filename_str = FILEPATH + "images/tilesets/blocks/hardmode.png";
    hard_mode_tile = QPixmap(QString(filename_str.c_str()));

    filename_str = FILEPATH + std::string("/images/tilesets/") + Mediator::get_instance()->getPallete();
    if (filename_str.length() <= 0) {
        return;
    }
    tileset_image = QPixmap(filename_str.c_str());
    if (tileset_image.isNull() == false) {
        QBitmap mask = tileset_image.createMaskFromColor(QColor(75, 125, 125), Qt::MaskInColor);
        tileset_image.setMask(mask);
    } else {
        tileset_bitmap.clear();
    }

    std::string bg1_filename = FILEPATH +"/images/map_backgrounds/"+ Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[0].filename;
    if (bg1_filename.length() > 0) {
        if (bg1_filename.find(".png") != std::string::npos) {
            bg1_image = QPixmap(bg1_filename.c_str());
            QBitmap bg1_mask = bg1_image.createMaskFromColor(QColor(75, 125, 125), Qt::MaskInColor);
            bg1_image.setMask(bg1_mask);

        } else {
            bg1_image = QPixmap();
        }
    } else {
        bg1_image = QPixmap();
    }

    std::string fg_filename = FILEPATH +"/images/map_backgrounds/"+ Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[1].filename;
    if (fg_filename.length() > 0) {
        if (fg_filename.find(".png") != std::string::npos) {
            fg_layer__image = QPixmap(fg_filename.c_str());
            QBitmap bg1_mask = fg_layer__image.createMaskFromColor(QColor(75, 125, 125), Qt::MaskInColor);
            fg_layer__image.setMask(bg1_mask);

        } else {
            fg_layer__image = QPixmap();
        }
    } else {
        fg_layer__image = QPixmap();
    }
    fg_opacity = (float)Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[1].gfx/100;
}




void EditorArea::paintEvent(QPaintEvent *) {

    if (Mediator::get_instance()->currentStage < 0) {
        return;
    }

    if (tileset_image.isNull()) {
        return;
    }


    int i, j, pos;
    QPainter painter(this);
    QLineF line;
    QString filename;


	// draw background-color
    if (Mediator::get_instance()->show_background_color == true) {
        //painter.setPen(QColor(Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.r, Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.g, Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.b, 255));
        painter.fillRect(QRectF(0.0, 0.0, MAP_W*TILESIZE*Mediator::get_instance()->zoom, MAP_H*TILESIZE*Mediator::get_instance()->zoom), QColor(Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.r, Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.g, Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.b, 255));
	}
	// draw background1
    std::string bg1_filename(Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[0].filename);
    if (Mediator::get_instance()->show_bg1 == true) {
        if (!bg1_image.isNull()) {
            int max_repeat = ((MAP_W*16)/bg1_image.width())*Mediator::get_instance()->zoom+1;
            for (int k=0; k<max_repeat; k++) {
                QRectF pos_source(QPoint(0, 0), QSize(bg1_image.width(), bg1_image.height()));
                QRectF pos_dest(QPoint(k*bg1_image.width()*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[0].adjust_y*Mediator::get_instance()->zoom), QSize(bg1_image.width()*Mediator::get_instance()->zoom, bg1_image.height()*Mediator::get_instance()->zoom));
                painter.drawPixmap(pos_dest, bg1_image, pos_source);
            }
        }
    }





    // DRAW TILES //
    for (i=0; i<MAP_W; i++) {
        for (j=0; j<MAP_H; j++) {

            if (Mediator::get_instance()->show_tileset_flag == true) {
                // level one
                if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x >= 0 && Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y >= 0) {
                    QRectF target(QPoint(i*16*Mediator::get_instance()->zoom, j*16*Mediator::get_instance()->zoom), QSize(16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom));
                    QRectF source(QPoint((Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x*16), (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y*16)), QSize(16, 16));
                    painter.drawPixmap(target, tileset_image, source);
                // animated tiles
                } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x < -1 && Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y == 0) {
                    int anim_tile_id = (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x*-1) - 2;

                    // check that the vector contains this
                    if (Mediator::get_instance()->anim_block_list.size() > 0 && anim_tile_id < Mediator::get_instance()->anim_block_list.size()) {
                        CURRENT_FILE_FORMAT::file_anim_block anim_tile = Mediator::get_instance()->anim_block_list.at(anim_tile_id);
                        QString anim_tile_filename = QString(FILEPATH.c_str()) + QString("/images/tilesets/anim/") + QString(anim_tile.filename);

                        QPixmap anim_image(anim_tile_filename);

                        if (anim_image.isNull() == false) {

                            QBitmap anim_image_mask = anim_image.createMaskFromColor(QColor(75, 125, 125), Qt::MaskInColor);
                            anim_image.setMask(anim_image_mask);


                            QRectF target(QPoint(i*TILESIZE*Mediator::get_instance()->zoom, j*TILESIZE*Mediator::get_instance()->zoom), QSize(TILESIZE*Mediator::get_instance()->zoom, TILESIZE*Mediator::get_instance()->zoom));
                            QRectF source(QPoint(0, 0), QSize(TILESIZE, TILESIZE));

                            painter.drawPixmap(target, anim_image, source);
                            // draw an green border border to indicate anim tile
                            QPen pen(QColor(0, 200, 0), 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
                            painter.setPen(pen);

                            if (Mediator::get_instance()->show_grid) {
                                int anim_tile_x = i * TILESIZE * Mediator::get_instance()->zoom; // minus tilesize is because width starts in 1, not zero
                                int anim_tile_y = j * TILESIZE *Mediator::get_instance()->zoom;
                                painter.drawLine(anim_tile_x, anim_tile_y, anim_tile_x+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_y);
                                painter.drawLine(anim_tile_x, anim_tile_y, anim_tile_x, anim_tile_y+(TILESIZE*Mediator::get_instance()->zoom));
                                painter.drawLine(anim_tile_x, anim_tile_y+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_x+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_y+(TILESIZE*Mediator::get_instance()->zoom));
                                painter.drawLine(anim_tile_x+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_y, anim_tile_x+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_y+(TILESIZE*Mediator::get_instance()->zoom));
                            }
                        }
                    }
                }

                // level 3
                painter.setOpacity(0.5);
                if (Mediator::get_instance()->layerLevel > 1) {
                    painter.setOpacity(1.0);
                }

                int tile3x = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x;
                int tile3y = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.y;

                if (Mediator::get_instance()->currentStage == 7 && Mediator::get_instance()->currentMap == 0 && tile3x != -1 && tile3y != -1) {
                    std::cout << ">> tile3[" << tile3x << "]["  << tile3y << "]" << std::endl;
                }

                if (tile3x > -1 && tile3y != -1) {
                    QRectF target(QPoint(i*TILESIZE*Mediator::get_instance()->zoom, j*TILESIZE*Mediator::get_instance()->zoom), QSize(TILESIZE*Mediator::get_instance()->zoom, TILESIZE*Mediator::get_instance()->zoom));
                    QRectF source(QPoint((Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x*TILESIZE), (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.y*16)), QSize(16, 16));
                    painter.drawPixmap(target, tileset_image, source);
                // animated tiles
                } else if (tile3x < -1 && tile3y == 0) {
                    int anim_tile_id = (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x*-1) - 2;

                    // check that the vector contains this
                    if (Mediator::get_instance()->anim_block_list.size() > 0 && anim_tile_id < Mediator::get_instance()->anim_block_list.size()) {
                        CURRENT_FILE_FORMAT::file_anim_block anim_tile = Mediator::get_instance()->anim_block_list.at(anim_tile_id);
                        QString anim_tile_filename = QString(FILEPATH.c_str()) + QString("/images/tilesets/anim/") + QString(anim_tile.filename);

                        QPixmap anim_image(anim_tile_filename);

                        if (anim_image.isNull() == false) {

                            QBitmap anim_image_mask = anim_image.createMaskFromColor(QColor(75, 125, 125), Qt::MaskInColor);
                            anim_image.setMask(anim_image_mask);

                            QRectF target(QPoint(i*TILESIZE*Mediator::get_instance()->zoom, j*TILESIZE*Mediator::get_instance()->zoom), QSize(TILESIZE*Mediator::get_instance()->zoom, TILESIZE*Mediator::get_instance()->zoom));
                            QRectF source(QPoint(0, 0), QSize(TILESIZE, TILESIZE));

                            painter.drawPixmap(target, anim_image, source);


                            // draw an green border border to indicate anim tile
                            if (Mediator::get_instance()->show_grid) {
                                QPen pen(QColor(0, 200, 0), 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
                                painter.setPen(pen);

                                int anim_tile_x = i * TILESIZE * Mediator::get_instance()->zoom; // minus tilesize is because width starts in 1, not zero
                                int anim_tile_y = j * TILESIZE *Mediator::get_instance()->zoom;
                                painter.drawLine(anim_tile_x, anim_tile_y, anim_tile_x+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_y);
                                painter.drawLine(anim_tile_x, anim_tile_y, anim_tile_x, anim_tile_y+(TILESIZE*Mediator::get_instance()->zoom));
                                painter.drawLine(anim_tile_x, anim_tile_y+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_x+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_y+(TILESIZE*Mediator::get_instance()->zoom));
                                painter.drawLine(anim_tile_x+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_y, anim_tile_x+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_y+(TILESIZE*Mediator::get_instance()->zoom));
                            }
                        }
                    }
                    }
                painter.setOpacity(1.0);


                // EASY-mode tiles
                if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == TERRAIN_EASYMODEBLOCK) {

                    QRectF target(QPoint(i*16*Mediator::get_instance()->zoom, j*16*Mediator::get_instance()->zoom), QSize(16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom));
                    QRectF source(QPoint(0, 0), QSize(16, 16));
                    painter.drawPixmap(target, easy_mode_tile, source);

                    painter.setBrush(QColor(220, 210, 50, 100));
                    painter.drawRect(i*16*Mediator::get_instance()->zoom, j*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom);

                // HARD-mode tiles
                } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == TERRAIN_HARDMODEBLOCK) {
                    QRectF target(QPoint(i*16*Mediator::get_instance()->zoom, j*16*Mediator::get_instance()->zoom), QSize(16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom));
                    QRectF source(QPoint(0, 0), QSize(16, 16));
                    painter.drawPixmap(target, hard_mode_tile, source);

                    painter.setBrush(QColor(190, 36, 230, 100));
                    painter.drawRect(i*16*Mediator::get_instance()->zoom, j*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom);
                }
            }


            // locked areas, stairs, doors, etc
            if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked != 0 && Mediator::get_instance()->editTool == EDITMODE_LOCK) {
                    // translucid rectangle
                    //painter.setBrush(QColor(255, 0, 0, 30));
                    //painter.drawRect(i*16, j*16, 16, 16);
                    // red border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(255, 0, 0, 255));
                    painter.drawRect(i*16*Mediator::get_instance()->zoom, j*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom);
                    // terrain type icon
                    QString terrainIcon;
                    QResource::registerResource("resources/icons/icons.qrc");
                    if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 1) {
                        terrainIcon = QString::fromUtf8(":/toolbar_icons/Lock"); // solid
                    } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 2) {
                        terrainIcon = QString(":/toolbar_icons/stairs.png"); // stairs
                    } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 3) {
                        terrainIcon = QString(":/toolbar_icons/object-order-lower.png"); // door
                    } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 4) {
                        terrainIcon = QString(":/toolbar_icons/edit-delete.png"); // spikes
                    } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 5) {
                        terrainIcon = QString(":/toolbar_icons/flag-blue.png"); // water
                    } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 6) {
                        terrainIcon = QString(":/toolbar_icons/flag-green.png"); // ice
                    } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 7) {
                        terrainIcon = QString(":/toolbar_icons/arrow-left.png"); // move left
                    } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 8) {
                        terrainIcon = QString(":/toolbar_icons/arrow-right.png"); // move right
                    } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 9) {
                        terrainIcon = QString(":/toolbar_icons/arrow-down.png"); // move right
                    } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 10) {
                        terrainIcon = QString(":/toolbar_icons/system-switch-user.png"); // move right
                    } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 11) {
                        terrainIcon = QString(":/toolbar_icons/Save"); // move right
                    } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 12) {
                        terrainIcon = QString(":/toolbar_icons/draw-polygon.png"); // easy block
                    } else if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 13) {
                        terrainIcon = QString(":/toolbar_icons/draw-square-inverted-corners.png"); // hard block

                    }
                    QPixmap terrainImage(terrainIcon);
                    if (terrainImage.isNull()) {
                        printf("ERROR: EditorArea::paintEvent - terrainType - Could not load image file '%s'\n", qPrintable(terrainIcon));
                    } else {
                        terrainIcon.resize(16);
                        painter.setOpacity(0.7);
                        QRectF target(QPoint(i*16*Mediator::get_instance()->zoom, j*16*Mediator::get_instance()->zoom), QSize(16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom));
                        QRectF source(QPoint(0, 0), QSize(terrainImage.width (), terrainImage.height ()));
                        painter.drawPixmap(target, terrainImage, source);
                    }
                    painter.setOpacity(1.0);
            }
        }
    }




    // DRAW LINKS //
    if (Mediator::get_instance()->show_teleporters_flag == true) {
        // draw links
        int link_type = Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].type;
        //printf("editoMode: %d, EDITMODE_NORMAL: %d, editTool: %d, EDITMODE_LINK_DEST: %d\n", Mediator::get_instance()->editMode, EDITMODE_NORMAL, Mediator::get_instance()->editTool, EDITMODE_LINK_DEST);
        if (Mediator::get_instance()->editMode == EDITMODE_LINK && (Mediator::get_instance()->editTool == EDITMODE_LINK_DEST || Mediator::get_instance()->editTool == EDITMODE_LINK)) {
            for (int i=0; i<STAGE_MAX_LINKS; i++) {
                // link is from and to the same map, draw in a different color
                if (Mediator::get_instance()->currentMap == Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_origin) {
                    // translucid blue rectangle
                    if (link_type == LINK_TELEPORTER || link_type == LINK_TELEPORT_LEFT_LOCK || link_type == LINK_TELEPORT_RIGHT_LOCK || link_type == LINK_FADE_TELEPORT) {
                        painter.setBrush(QColor(0, 255, 0, 180));
                    } else {
                        // if origin and destiny are the same map, use yellow
                        if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_origin == Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_destiny) {
                            painter.setBrush(QColor(231, 209, 58, 180));
                        } else {
                            painter.setBrush(QColor(0, 0, 255, 180));
                        }
                    }
                    painter.drawRect(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.y *16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom*Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].size, 16*Mediator::get_instance()->zoom);
                    // blue border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(0, 0, 255, 255));
                    painter.drawRect(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.y*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom*Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].size, 16*Mediator::get_instance()->zoom);
                    painter.setPen(QColor(255, 255, 255, 255));
                    painter.drawText(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.x*16*Mediator::get_instance()->zoom, (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.y+1)*16*Mediator::get_instance()->zoom, QString::number(i));
                }
                if (Mediator::get_instance()->currentMap == Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_destiny) {
                    // translucid cyan rectangle
                    if (link_type == LINK_TELEPORTER || link_type == LINK_TELEPORT_LEFT_LOCK || link_type == LINK_TELEPORT_RIGHT_LOCK || link_type == LINK_FADE_TELEPORT) {
                        painter.setBrush(QColor(60, 160, 60, 180));
                    } else {
                        // if origin and destiny are the same map, use yellow
                        if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_origin == Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_destiny) {
                            painter.setBrush(QColor(184, 171, 84, 180));
                        } else {
                            painter.setBrush(QColor(0, 255, 255, 180));
                        }
                    }
                    painter.drawRect(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_destiny.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_destiny.y*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom*Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].size, 16*Mediator::get_instance()->zoom);
                    // cyan border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(0, 255, 255, 255));
                    painter.drawRect(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_destiny.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_destiny.y*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom*Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].size, 16*Mediator::get_instance()->zoom);
                    painter.setPen(QColor(0, 0, 0, 255));
                    painter.drawText(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_destiny.x*16*Mediator::get_instance()->zoom, (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_destiny.y+1)*16*Mediator::get_instance()->zoom, QString::number(i));
                }
            }
        }
    }


    // DRAW ENEMIES BACKGROUNDS //
    std::map<int, int> npc_bg_width_map;
    if (Mediator::get_instance()->show_npcs_flag == true) {
        /// draw NPCs
        for (int i=0; i<Mediator::get_instance()->maps_data_npc_list.size(); i++) {
            if (Mediator::get_instance()->maps_data_npc_list[i].stage_id != Mediator::get_instance()->currentStage || Mediator::get_instance()->maps_data_npc_list[i].map_id != Mediator::get_instance()->currentMap) {
                continue; // only show enemies from current stage/map
            }
            if (Mediator::get_instance()->maps_data_npc_list[i].difficulty_mode == DIFFICULTY_MODE_GREATER && Mediator::get_instance()->maps_data_npc_list[i].difficulty_level > Mediator::get_instance()->currentDifficulty) {
                continue; // only show enemies with equal or lower difficulty
            } else if (Mediator::get_instance()->maps_data_npc_list[i].difficulty_mode == DIFFICULTY_MODE_EQUAL && Mediator::get_instance()->maps_data_npc_list[i].difficulty_level != Mediator::get_instance()->currentDifficulty) {
                continue;
            }

            int npc_id = Mediator::get_instance()->maps_data_npc_list[i].id_npc;
            if (npc_id >= Mediator::get_instance()->enemy_list.size() || npc_id < 0) {
                Mediator::get_instance()->maps_data_npc_list[i].id_npc = -1;
                continue;
            }


            std::string npc_bg_file(Mediator::get_instance()->enemy_list.at(npc_id).bg_graphic_filename);
            if (npc_bg_file.length() > 0) {
                std::string _bg_graphic_filename = FILEPATH + "/images/sprites/enemies/backgrounds/" + npc_bg_file;
                QPixmap bg_image(_bg_graphic_filename.c_str());

                // calculate total image size of background exists
                if (!bg_image.isNull()) {
                    int total_w = bg_image.width();
                    npc_bg_width_map.insert(std::pair<int, int>(npc_id, bg_image.width()));
                    int total_h = bg_image.height();
                    QBitmap enemy_bg_mask = bg_image.createMaskFromColor(QColor(75, 125, 125), Qt::MaskInColor);
                    bg_image.setMask(enemy_bg_mask);
                    QRectF bg_target(QPoint(Mediator::get_instance()->maps_data_npc_list[i].start_point.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data_npc_list[i].start_point.y*16*Mediator::get_instance()->zoom), QSize(total_w*Mediator::get_instance()->zoom, total_h*Mediator::get_instance()->zoom));
                    QRectF bg_source(QRectF(QPoint(0, 0), QSize(bg_image.width(), bg_image.height())));

                    int direction = Mediator::get_instance()->maps_data_npc_list[i].direction;
                    // TODO: store this background to optimize things
                    if (direction == ANIM_DIRECTION_LEFT) {
                        QImage temp_img(_bg_graphic_filename.c_str());
                        QImage mirror_image = temp_img.copy(0, 0, total_w, total_h);
                        mirror_image = mirror_image.mirrored(true, false);
                        QPixmap mirror_image_pixmap = QPixmap().fromImage(mirror_image);

                        QBitmap mirror_enemy_bg_mask = mirror_image_pixmap.createMaskFromColor(QColor(75, 125, 125), Qt::MaskInColor);
                        mirror_image_pixmap.setMask(mirror_enemy_bg_mask);
                        painter.drawPixmap(bg_target, mirror_image_pixmap, bg_source);
                    } else {
                        painter.drawPixmap(bg_target, bg_image, bg_source);
                    }
                }
            }
        }
    }

    // DRAW ENEMIES //
    if (Mediator::get_instance()->show_npcs_flag == true) {
        /// draw NPCs
        for (unsigned int i=0; i<Mediator::get_instance()->maps_data_npc_list.size(); i++) {
            if (Mediator::get_instance()->maps_data_npc_list[i].stage_id != Mediator::get_instance()->currentStage || Mediator::get_instance()->maps_data_npc_list[i].map_id != Mediator::get_instance()->currentMap) {
                continue; // only show enemies from current stage/map
            }
            if (Mediator::get_instance()->maps_data_npc_list[i].difficulty_mode == DIFFICULTY_MODE_GREATER && Mediator::get_instance()->maps_data_npc_list[i].difficulty_level > Mediator::get_instance()->currentDifficulty) {
                continue; // only show enemies with equal or lower difficulty
            } else if (Mediator::get_instance()->maps_data_npc_list[i].difficulty_mode == DIFFICULTY_MODE_EQUAL && Mediator::get_instance()->maps_data_npc_list[i].difficulty_level != Mediator::get_instance()->currentDifficulty) {
                continue;
            }

            int npc_id = Mediator::get_instance()->maps_data_npc_list[i].id_npc;
            if (npc_id >= Mediator::get_instance()->enemy_list.size() || npc_id < 0) {
                Mediator::get_instance()->maps_data_npc_list[i].id_npc = -1;
                continue;
            }
            std::string filename = FILEPATH + "/images/sprites/enemies/" + Mediator::get_instance()->enemy_list.at(npc_id).graphic_filename;
            if (filename.length() <= 0) {
                continue;
            }


            QPixmap temp_image(filename.c_str());
            if (!temp_image.isNull()) {

                QBitmap enemy_mask = temp_image.createMaskFromColor(QColor(75, 125, 125), Qt::MaskInColor);
                temp_image.setMask(enemy_mask);

                int total_w = Mediator::get_instance()->enemy_list.at(npc_id).frame_size.width*Mediator::get_instance()->zoom;
                int total_h = Mediator::get_instance()->enemy_list.at(npc_id).frame_size.height*Mediator::get_instance()->zoom;
                int sprite_adjust_x = Mediator::get_instance()->enemy_list.at(npc_id).sprites_pos_bg.x;
                int sprite_adjust_y = Mediator::get_instance()->enemy_list.at(npc_id).sprites_pos_bg.y;

                QRectF target(QPoint((Mediator::get_instance()->maps_data_npc_list[i].start_point.x*16+sprite_adjust_x)*Mediator::get_instance()->zoom, (Mediator::get_instance()->maps_data_npc_list[i].start_point.y*16+sprite_adjust_y)*Mediator::get_instance()->zoom), QSize(total_w, total_h));
                QRectF source;
                if (Mediator::get_instance()->maps_data_npc_list[i].direction != ANIM_DIRECTION_RIGHT || temp_image.height() <= Mediator::get_instance()->enemy_list.at(npc_id).frame_size.height) {
                    source = QRectF(QPoint(0, 0), QSize(Mediator::get_instance()->enemy_list.at(npc_id).frame_size.width, Mediator::get_instance()->enemy_list.at(npc_id).frame_size.height));
                } else {
                    source = QRectF(QPoint(0, Mediator::get_instance()->enemy_list.at(npc_id).frame_size.height), QSize(Mediator::get_instance()->enemy_list.at(npc_id).frame_size.width, Mediator::get_instance()->enemy_list.at(npc_id).frame_size.height));
                }
                if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.id_npc == npc_id) {
                    // translucid green rectangle
                    painter.setBrush(QColor(0, 255, 0, 180));
                    painter.drawRect(target);
                } else if (Mediator::get_instance()->enemy_list.at(npc_id).is_boss) {
                    // translucid orange rectangle
                    painter.setBrush(QColor(255, 128, 35, 200));
                    painter.drawRect(target);
                } else if (Mediator::get_instance()->enemy_list.at(npc_id).is_sub_boss) {
                    // translucid bright-orange rectangle
                    painter.setBrush(QColor(241, 188, 87, 200));
                    painter.drawRect(target);
                }
                if (Mediator::get_instance()->game_data.final_boss_id == npc_id) {
                    // golden border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(255, 215, 0, 255));
                    painter.drawRect(target);
                }

                int direction = Mediator::get_instance()->maps_data_npc_list[i].direction;
                if (direction == ANIM_DIRECTION_LEFT) {
                    QImage temp_img(filename.c_str());
                    QImage mirror_image = temp_img.copy(source.x(), source.y(), source.width(), source.height());
                    mirror_image = mirror_image.mirrored(true, false);
                    QPixmap mirror_image_pixmap = QPixmap().fromImage(mirror_image);

                    QBitmap mirror_enemy_bg_mask = mirror_image_pixmap.createMaskFromColor(QColor(75, 125, 125), Qt::MaskInColor);
                    mirror_image_pixmap.setMask(mirror_enemy_bg_mask);

                    if (npc_bg_width_map.find(npc_id) != npc_bg_width_map.end()) {
                        int original_pos_x = Mediator::get_instance()->maps_data_npc_list[i].start_point.x*TILESIZE;
                        int bg_image_width = npc_bg_width_map.find(npc_id)->second;
                        int graph_diff_x = bg_image_width - Mediator::get_instance()->enemy_list.at(npc_id).sprites_pos_bg.x - Mediator::get_instance()->enemy_list.at(npc_id).frame_size.width;
                        //std::cout << "char[" << name << "], original_pos_x[" << original_pos_x << "], pos.x[" << pos.x << "], background_pos.x[" << background_pos.x << "], graph_diff_x[" << graph_diff_x << "]" << std::endl;
                        int new_pos_x = original_pos_x + graph_diff_x;
                        //std::cout << "NPC[" << Mediator::get_instance()->enemy_list.at(npc_id).name << "], bg_image_width[" << bg_image_width << "], new_pos_x[" << new_pos_x << "], graph_diff_x[" << graph_diff_x << "], bg_img.w[" << bg_image_width << "], bg_pos.x[" << Mediator::get_instance()->enemy_list.at(npc_id).sprites_pos_bg.x << "]" << std::endl;
                        target = QRectF(QPoint((new_pos_x)*Mediator::get_instance()->zoom, (Mediator::get_instance()->maps_data_npc_list[i].start_point.y*16+sprite_adjust_y)*Mediator::get_instance()->zoom), QSize(total_w, total_h));
                    }
                    painter.drawPixmap(target, mirror_image_pixmap, source);
                    if (Mediator::get_instance()->show_rects_flag == true) {
                        painter.setBrush(QColor(255, 255, 255, 180));
                        painter.drawRect(target.left(), target.top(), target.width(), target.height());
                    }
                } else {
                    painter.drawPixmap(target, temp_image, source);
                    if (Mediator::get_instance()->show_rects_flag == true) {
                        painter.setBrush(QColor(255, 255, 255, 180));
                        painter.drawRect(target.left(), target.top(), target.width(), target.height());
                    }
                }
            }
        }
    }


    // DRAW OBJECTS //
    if (Mediator::get_instance()->show_objects_flag == true) {
        /// draw objects
        for (unsigned int i=0; i<Mediator::get_instance()->maps_data_object_list.size(); i++) {
            int obj_stage_id = (int)Mediator::get_instance()->maps_data_object_list[i].stage_id;
            int obj_map_id = (int)Mediator::get_instance()->maps_data_object_list[i].map_id;

            if (obj_stage_id != Mediator::get_instance()->currentStage || obj_map_id != Mediator::get_instance()->currentMap) {
                continue;
            }

            if (Mediator::get_instance()->maps_data_object_list[i].difficulty_mode == DIFFICULTY_MODE_GREATER && Mediator::get_instance()->maps_data_object_list[i].difficulty_level > Mediator::get_instance()->currentDifficulty) {
                continue; // only show enemies with equal or lower difficulty
            } else if (Mediator::get_instance()->maps_data_object_list[i].difficulty_mode == DIFFICULTY_MODE_EQUAL && Mediator::get_instance()->maps_data_object_list[i].difficulty_level != Mediator::get_instance()->currentDifficulty) {
                continue;
            }

            int obj_id = (int)Mediator::get_instance()->maps_data_object_list[i].id_object;


            if (obj_id != -1) {
                std::string filename = FILEPATH + "/images/sprites/objects/" + Mediator::get_instance()->object_list.at(obj_id).graphic_filename;
                QPixmap temp_image(filename.c_str());
                if (temp_image.isNull()) {
                    painter.setBrush(QColor(255, 255, 255, 180));
                    painter.drawRect(Mediator::get_instance()->maps_data_object_list[i].start_point.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data_object_list[i].start_point.y*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom);
                } else {

                    QBitmap mask = temp_image.createMaskFromColor(QColor(75, 125, 125), Qt::MaskInColor);
                    temp_image.setMask(mask);

                    int obj_type = Mediator::get_instance()->object_list.at(obj_id).type;
                    int obj_direction = Mediator::get_instance()->maps_data_object_list[i].direction;
                    if (obj_type == OBJ_RAY_HORIZONTAL) {
                        int graphic_pos_x = 0;
                        int graphic_pos_y = Mediator::get_instance()->object_list.at(obj_id).size.height;
                        if (obj_direction == ANIM_DIRECTION_LEFT) {
                            graphic_pos_x = Mediator::get_instance()->object_list.at(obj_id).size.width - TILESIZE;
                            graphic_pos_y = 0;
                        }
                        QRectF target(QPoint(Mediator::get_instance()->maps_data_object_list[i].start_point.x*TILESIZE*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data_object_list[i].start_point.y*TILESIZE*Mediator::get_instance()->zoom), QSize(TILESIZE*Mediator::get_instance()->zoom, Mediator::get_instance()->object_list.at(obj_id).size.height*Mediator::get_instance()->zoom));
                        QRectF source(QPoint(graphic_pos_x, graphic_pos_y), QSize(TILESIZE, Mediator::get_instance()->object_list.at(obj_id).size.height));
                        painter.drawPixmap(target, temp_image, source);
                    } else if (obj_type == OBJ_RAY_VERTICAL) {
                        int graphic_pos_y = Mediator::get_instance()->object_list.at(obj_id).size.height - TILESIZE;
                        QRectF target(QPoint(Mediator::get_instance()->maps_data_object_list[i].start_point.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data_object_list[i].start_point.y*16*Mediator::get_instance()->zoom), QSize(Mediator::get_instance()->object_list.at(obj_id).size.width*Mediator::get_instance()->zoom, Mediator::get_instance()->object_list.at(obj_id).size.height*Mediator::get_instance()->zoom));
                        QRectF source(QPoint(0, graphic_pos_y), QSize(Mediator::get_instance()->object_list.at(obj_id).size.width, Mediator::get_instance()->object_list.at(obj_id).size.height));
                        painter.drawPixmap(target, temp_image, source);
                    } else if (obj_type == OBJ_DEATHRAY_HORIZONTAL && obj_direction == ANIM_DIRECTION_LEFT) {
                        QRectF target(QPoint(Mediator::get_instance()->maps_data_object_list[i].start_point.x*TILESIZE*Mediator::get_instance()->zoom - (Mediator::get_instance()->object_list.at(obj_id).size.width-TILESIZE)*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data_object_list[i].start_point.y*16*Mediator::get_instance()->zoom), QSize(Mediator::get_instance()->object_list.at(obj_id).size.width*Mediator::get_instance()->zoom, Mediator::get_instance()->object_list.at(obj_id).size.height*Mediator::get_instance()->zoom));
                        QRectF source(QPoint(0, 0), QSize(Mediator::get_instance()->object_list.at(obj_id).size.width, Mediator::get_instance()->object_list.at(obj_id).size.height));
                        painter.drawPixmap(target, temp_image, source);
                    } else if (obj_type == OBJ_TIMED_BOMB) {
                        int bomb_direction =Mediator::get_instance()->maps_data_object_list[i].direction;
                        int y_graph_adjust = (bomb_direction * Mediator::get_instance()->object_list.at(obj_id).size.height)*2;
                        std::cout << "BOMB[" << i << "].direction[" << bomb_direction << "], y_graph_adjust[" << y_graph_adjust << "]" << std::endl;
                        QRectF target(QPoint(Mediator::get_instance()->maps_data_object_list[i].start_point.x*TILESIZE*Mediator::get_instance()->zoom - (Mediator::get_instance()->object_list.at(obj_id).size.width-TILESIZE)*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data_object_list[i].start_point.y*16*Mediator::get_instance()->zoom), QSize(Mediator::get_instance()->object_list.at(obj_id).size.width*Mediator::get_instance()->zoom, Mediator::get_instance()->object_list.at(obj_id).size.height*Mediator::get_instance()->zoom));
                        QRectF source(QPoint(0, y_graph_adjust), QSize(Mediator::get_instance()->object_list.at(obj_id).size.width, Mediator::get_instance()->object_list.at(obj_id).size.height));
                        painter.drawPixmap(target, temp_image, source);
                    } else {
                        QRectF target(QPoint(Mediator::get_instance()->maps_data_object_list[i].start_point.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data_object_list[i].start_point.y*16*Mediator::get_instance()->zoom), QSize(Mediator::get_instance()->object_list.at(obj_id).size.width*Mediator::get_instance()->zoom, Mediator::get_instance()->object_list.at(obj_id).size.height*Mediator::get_instance()->zoom));
                        QRectF source = QRectF(QPoint(0, 0), QSize(Mediator::get_instance()->object_list.at(obj_id).size.width, Mediator::get_instance()->object_list.at(obj_id).size.height));
                        if (obj_direction == ANIM_DIRECTION_RIGHT && temp_image.height() >= Mediator::get_instance()->object_list.at(obj_id).size.height*2) {
                            source = QRectF(QPoint(0, Mediator::get_instance()->object_list.at(obj_id).size.height), QSize(Mediator::get_instance()->object_list.at(obj_id).size.width, Mediator::get_instance()->object_list.at(obj_id).size.height));
                        }
                        //std::cout << "OBJECT[" << i << "], ID[" << (int)Mediator::get_instance()->maps_data_object_list.at(i).id_object << "] target[" << target.left() << "][" << target.top() << "], source.w[" << source.width() << "], source.h[" << source.height() << "]" << std::endl;
                        //painter.setBrush(QColor(255, 255, 255, 180));
                        if (Mediator::get_instance()->show_rects_flag == true) {
                            painter.setBrush(QColor(255, 255, 255, 180));
                            painter.drawRect(Mediator::get_instance()->maps_data_object_list[i].start_point.x*TILESIZE*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data_object_list[i].start_point.y*TILESIZE*Mediator::get_instance()->zoom, Mediator::get_instance()->object_list.at(obj_id).size.width*Mediator::get_instance()->zoom, Mediator::get_instance()->object_list.at(obj_id).size.height*Mediator::get_instance()->zoom);
                        }
                        painter.drawPixmap(target, temp_image, source);
                    }
                }
                // draw object-teleporter origin
                if (Mediator::get_instance()->maps_data_object_list[i].map_dest != -1) {
                    int obj_w = Mediator::get_instance()->object_list.at(obj_id).size.width;
                    int obj_h = Mediator::get_instance()->object_list.at(obj_id).size.height;
                    int dest_x = Mediator::get_instance()->maps_data_object_list[i].start_point.x*TILESIZE*Mediator::get_instance()->zoom + (obj_w/2)*Mediator::get_instance()->zoom - TILESIZE/2;
                    int dest_y = Mediator::get_instance()->maps_data_object_list[i].start_point.y*TILESIZE*Mediator::get_instance()->zoom + (obj_h/2)*Mediator::get_instance()->zoom - TILESIZE/2;
                    if (Mediator::get_instance()->object_list.at(obj_id).type == OBJ_FINAL_BOSS_TELEPORTER) {
                        painter.setBrush(QColor(160, 60, 60, 180));
                    } else {
                        painter.setBrush(QColor(60, 160, 60, 180));
                    }
                    painter.drawEllipse(dest_x, dest_y, 16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom);
                    // cyan border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(0, 255, 255, 255));
                    painter.drawEllipse(dest_x, dest_y, 16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom);
                    painter.setPen(QColor(0, 0, 0, 255));
                    painter.drawText(dest_x+3, dest_y+TILESIZE-3, QString::number(i));
                }
            }
        }


        // DRAW OBJECT TELEPORT CIRCLES
        for (int k=0; k<FS_STAGE_MAX_MAPS; k++) {
            for (int m=0; m<Mediator::get_instance()->maps_data_object_list.size(); m++) {
                if (Mediator::get_instance()->maps_data_object_list[m].stage_id != Mediator::get_instance()->currentStage) {
                    continue; // only show enemies from current stage/map
                }
                CURRENT_FILE_FORMAT::file_map_object_v2 map_obj = Mediator::get_instance()->maps_data_object_list[m];
                int obj_id = map_obj.id_object;
                if (obj_id == -1 || obj_id >= Mediator::get_instance()->object_list.size()) { // old format style or invalid object
                    continue;
                }

                // draw teleport destiny links
                if (map_obj.map_dest == Mediator::get_instance()->currentMap) {

                    if (Mediator::get_instance()->object_list.at(obj_id).type == OBJ_FINAL_BOSS_TELEPORTER) {
                        painter.setBrush(QColor(160, 60, 60, 180));
                    } else {
                        painter.setBrush(QColor(60, 160, 60, 180));
                    }

                    painter.drawEllipse(map_obj.link_dest.x*16*Mediator::get_instance()->zoom, map_obj.link_dest.y*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom);
                    // cyan border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(0, 255, 255, 255));

                    painter.drawEllipse(map_obj.link_dest.x*16*Mediator::get_instance()->zoom, map_obj.link_dest.y*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom);

                    painter.setPen(QColor(0, 0, 0, 255));
                    painter.drawText(map_obj.link_dest.x*16*Mediator::get_instance()->zoom + 3*Mediator::get_instance()->zoom, (map_obj.link_dest.y+1)*16*Mediator::get_instance()->zoom -2*Mediator::get_instance()->zoom, QString::number(m));
                }
            }
        }
    }

    if (Mediator::get_instance()->show_grid) {
        // DRAW GRID //
        QPen pen(QColor(160, 160, 160), 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
        QPen pen_red(QColor(255, 0, 255), 2, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
        QPen pen_white(QColor(250, 250, 250), 2, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);
        for (i=1; i<MAP_W; i++) {
            pos = i*16*Mediator::get_instance()->zoom-1;
            //QLineF line(0, 800, 16, 800);
            // linhas horizontais
            line = QLineF(pos, 0, pos, MAP_H*16*Mediator::get_instance()->zoom-1);
            if (i % 20 == 0) {
                painter.setPen(pen_white);
                QLineF lineWhite = QLineF(pos+4, 0, pos+4, MAP_H*16*Mediator::get_instance()->zoom-1);
                painter.drawLine(lineWhite);
                painter.setPen(pen_red);
            } else {
                painter.setPen(pen);
            }
            painter.drawLine(line);
        }
        painter.setPen(pen);
        for (i=1; i<MAP_H; i++) {
            pos = i*16*Mediator::get_instance()->zoom-1;
            //QLineF line(0, 800, 16, 800);
            // linhas verticais
            line = QLineF(0, pos, MAP_W*16*Mediator::get_instance()->zoom-1, pos);
            painter.drawLine(line);
        }
    }

    // === FOREGROUND LAYER IMAGE == //
    if (Mediator::get_instance()->show_fg_layer == true) {
        if (!fg_layer__image.isNull()) {
            int max_repeat = ((MAP_W*16)/fg_layer__image.width())*Mediator::get_instance()->zoom+1;
            for (int k=0; k<max_repeat; k++) {
                QRectF pos_source(QPoint(0, 0), QSize(fg_layer__image.width(), fg_layer__image.height()));
                QRectF pos_dest(QPoint(k*fg_layer__image.width()*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[1].adjust_y*Mediator::get_instance()->zoom), QSize(fg_layer__image.width()*Mediator::get_instance()->zoom, fg_layer__image.height()*Mediator::get_instance()->zoom));
                painter.setOpacity(fg_opacity);
                painter.drawPixmap(pos_dest, fg_layer__image, pos_source);
                painter.setOpacity(1);
            }
        }
    }


    // === draw selection === //
    if (Mediator::get_instance()->editMode == EDITMODE_SELECT) {
        painter.setBrush(QColor(0, 0, 255, 180));
        painter.drawRect(selection_start_x*TILESIZE*Mediator::get_instance()->zoom, selection_start_y*TILESIZE*Mediator::get_instance()->zoom, abs(selection_current_x-selection_start_x)*TILESIZE*Mediator::get_instance()->zoom, abs(selection_current_y-selection_start_y)*TILESIZE*Mediator::get_instance()->zoom);
    }


    QSize resizeMe(MAP_W*16*Mediator::get_instance()->zoom, MAP_H*16*Mediator::get_instance()->zoom);
    this->resize(resizeMe);
    myParent->adjustSize();

}

void EditorArea::mouseMoveEvent(QMouseEvent *event) {
	QPoint pnt = event->pos();

    if (Mediator::get_instance()->editMode != EDITMODE_SELECT) {
        // forces "click" when moving
        if (editor_selectedTileX != pnt.x()/(16*Mediator::get_instance()->zoom) || editor_selectedTileY != pnt.y()/(16*Mediator::get_instance()->zoom)) {
            mousePressEvent(event);
        }
    } else {
        selection_current_x = pnt.x()/(TILESIZE*Mediator::get_instance()->zoom) + 1;
        selection_current_y = pnt.y()/(TILESIZE*Mediator::get_instance()->zoom) + 1;
        repaint();
    }
}

void EditorArea::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;

    if (numSteps > 0) {
        if (Mediator::get_instance()->zoom < 3) {
            Mediator::get_instance()->zoom++;
            repaint();
        }
    } else if (numSteps < 0) {
        if (Mediator::get_instance()->zoom > 1) {
            Mediator::get_instance()->zoom--;
            repaint();
        }
    }
}

void EditorArea::mousePressEvent(QMouseEvent *event) {
    if (mouse_released == false && (Mediator::get_instance()->editTool == EDITMODE_LINK || Mediator::get_instance()->editTool == EDITMODE_LINK_DEST || Mediator::get_instance()->editMode == EDITMODE_NPC || Mediator::get_instance()->editMode == EDITMODE_OBJECT || Mediator::get_instance()->editMode == EDITMODE_SET_BOSS || Mediator::get_instance()->editMode == EDITMODE_SET_SUBBOSS || Mediator::get_instance()->editMode == EDITMODE_ANIM_TILE)) {
		return;
	}

    QPoint pnt = event->pos();
    editor_selectedTileX = pnt.x()/(16*Mediator::get_instance()->zoom);
    editor_selectedTileY = pnt.y()/(16*Mediator::get_instance()->zoom);

    if (Mediator::get_instance()->editMode == EDITMODE_NORMAL) {
        if (Mediator::get_instance()->editTool == EDITMODE_NORMAL) {
            if (Mediator::get_instance()->layerLevel == 1) {
                Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.x = Mediator::get_instance()->getPalleteX();
                Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.y = Mediator::get_instance()->getPalleteY();
            } else if (Mediator::get_instance()->layerLevel > 1) {
                Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.x = Mediator::get_instance()->getPalleteX();
                Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.y = Mediator::get_instance()->getPalleteY();
			}
        } else if (Mediator::get_instance()->editTool == EDITMODE_ERASER) {
            if (Mediator::get_instance()->layerLevel == 1) {
                Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.x = -1;
                Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.y = -1;
            } else if (Mediator::get_instance()->layerLevel > 1) {
                Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.x = -1;
                Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.y = -1;
			}
        } else if (Mediator::get_instance()->editTool == EDITMODE_LOCK) {
            if (Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].locked == 0) {
                Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].locked = Mediator::get_instance()->terrainType;
			} else {
                Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].locked = 0;
			}

        } else if (Mediator::get_instance()->editTool == EDITMODE_FILL) {
			fill_area();

        }

    } else if (Mediator::get_instance()->editMode == EDITMODE_ANIM_TILE) {
        int tilex = -1;
        int tiley = -1;
        if (Mediator::get_instance()->editTool == EDITMODE_NORMAL) {
            // we set x as anim_tile.id -2 and y as zero
            int anim_tile_id = (Mediator::get_instance()->selectedAnimTileset*-1) - 2;

            if (anim_tile_id >= 0) {
                return;
            }
            int abs_anim_tile_id = abs(anim_tile_id) - 2;
            if (abs_anim_tile_id >= Mediator::get_instance()->anim_block_list.size()) {
                return;
            }
            tilex = anim_tile_id;
            tiley = 0;
        }
        if (Mediator::get_instance()->layerLevel == 1) {
            Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.x = tilex;
            Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.y = tiley;
        } else if (Mediator::get_instance()->layerLevel > 1) {
            Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.x = tilex;
            Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.y = tiley;
        }



    } else if (Mediator::get_instance()->editMode == EDITMODE_LINK) {
		// first click on origin link
        if (Mediator::get_instance()->editTool == EDITMODE_LINK && tempX == -1) {
			// checks if a link in this position already exits to remove it
			bool removed_link = false;
			for (int i=0; i<STAGE_MAX_LINKS; i++) {
                if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_origin == Mediator::get_instance()->currentMap) {
                    if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.x == editor_selectedTileX && Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.y == editor_selectedTileY) {
                        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_destiny = -1;
                        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_origin = -1;
						removed_link = true;
						break;
					}
                } else if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_destiny == Mediator::get_instance()->currentMap) {
                    if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_destiny.x == editor_selectedTileX && Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_destiny.y == editor_selectedTileY) {
                        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_destiny = -1;
                        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_origin = -1;
						removed_link = true;
						break;
					}
				}
			}
			if (removed_link == false) {
				// check if there is a slot free
				int link_n = -1;
				for (int i=0; i<STAGE_MAX_LINKS; i++) {
                    if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_destiny == -1 && Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_origin == -1) {
						link_n = i;
						break;
					}
				}
				if (link_n == -1) {
					QMessageBox msgBox;
					msgBox.setText("########### -> Error: there is no slot free to add a new link.");
					msgBox.exec();
					return;
				}
                // last boss room doesen't need destination
                tempX = editor_selectedTileX;
                tempY = editor_selectedTileY;
			}
        } else if (Mediator::get_instance()->editTool == EDITMODE_LINK_DEST) {
			int link_n = -1;
			for (int i=0; i<STAGE_MAX_LINKS; i++) {
                if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_destiny == -1 && Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_origin == -1) {
					link_n = i;
					break;
				}
			}
			if (link_n == -1) {
				QMessageBox msgBox;
				msgBox.setText("Error: there is no slot free to add a new link.");
				msgBox.exec();
				return;
			}
            Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].id_map_origin = link_map_origin;
            Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].pos_origin.x = link_pos_x;
            Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].pos_origin.y = link_pos_y;
            Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].size = link_size;
            Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].id_map_destiny = Mediator::get_instance()->currentMap;
            Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].pos_destiny.x = editor_selectedTileX;
            Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].pos_destiny.y = editor_selectedTileY;
			/// @TODO: this must come from editor properties
            Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].bidirecional = Mediator::get_instance()->link_bidi;
            Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].is_door = Mediator::get_instance()->link_is_door;
            Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].type = Mediator::get_instance()->link_type;

            Mediator::get_instance()->editTool = EDITMODE_LINK;
			repaint();
            QApplication::setOverrideCursor(Qt::ArrowCursor);
			return;
		}



    } else if (Mediator::get_instance()->editMode == EDITMODE_SET_BOSS) {
        // search if there is an existing NPC in ths position, and if yes, set as boss
        for (int i=0; i<Mediator::get_instance()->maps_data_npc_list.size(); i++) {
            // only show enemies from current stage/map
            if (Mediator::get_instance()->maps_data_npc_list[i].stage_id != Mediator::get_instance()->currentStage || Mediator::get_instance()->maps_data_npc_list[i].map_id != Mediator::get_instance()->currentMap) {
                continue;
            }
            int npc_id = Mediator::get_instance()->maps_data_npc_list[i].id_npc;
            if (npc_id != -1 && Mediator::get_instance()->maps_data_npc_list[i].start_point.x == editor_selectedTileX && Mediator::get_instance()->maps_data_npc_list[i].start_point.y == editor_selectedTileY) {
                Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.id_npc = Mediator::get_instance()->maps_data_npc_list[i].id_npc;
                sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.name, "%s", Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->maps_data_npc_list[i].id_npc).name);
                break;
            }
        }


    } else if (Mediator::get_instance()->editMode == EDITMODE_SET_SUBBOSS) {
        // search if there is an existing NPC in ths position, and if yes, set as sub-boss
        for (int i=0; i<Mediator::get_instance()->maps_data_npc_list.size(); i++) {
            if (Mediator::get_instance()->maps_data_npc_list[i].stage_id != Mediator::get_instance()->currentStage || Mediator::get_instance()->maps_data_npc_list[i].map_id != Mediator::get_instance()->currentMap) {
                continue; // only show enemies from current stage/map
            }
            if (Mediator::get_instance()->maps_data_npc_list[i].id_npc != -1 && Mediator::get_instance()->maps_data_npc_list[i].start_point.x == editor_selectedTileX && Mediator::get_instance()->maps_data_npc_list[i].start_point.y == editor_selectedTileY) {
                Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->maps_data_npc_list[i].id_npc).is_sub_boss = !Mediator::get_instance()->enemy_list.at(Mediator::get_instance()->maps_data_npc_list[i].id_npc).is_sub_boss;
                break;
            }
        }


    } else if (Mediator::get_instance()->editMode == EDITMODE_NPC) {
		int found_npc = -1;
		// search if there is an existing NPC in ths position, and if yes, remove it
        for (int i=0; i<Mediator::get_instance()->maps_data_npc_list.size(); i++) {
            if (Mediator::get_instance()->maps_data_npc_list[i].stage_id != Mediator::get_instance()->currentStage || Mediator::get_instance()->maps_data_npc_list[i].map_id != Mediator::get_instance()->currentMap) {
                continue; // only show enemies from current stage/map
            }
            if (Mediator::get_instance()->maps_data_npc_list[i].id_npc != -1 && Mediator::get_instance()->maps_data_npc_list[i].start_point.x == editor_selectedTileX && Mediator::get_instance()->maps_data_npc_list[i].start_point.y == editor_selectedTileY) {
				found_npc = i;
				break;
			}
		}
		// clean old-format trash
        for (int i=0; i<Mediator::get_instance()->maps_data_npc_list.size(); i++) {
            if (Mediator::get_instance()->maps_data_npc_list[i].start_point.x == -1 && Mediator::get_instance()->maps_data_npc_list[i].start_point.y == -1) {
                Mediator::get_instance()->maps_data_npc_list[i].id_npc = -1;
			}
		}


        if (Mediator::get_instance()->editTool == EDITMODE_ERASER && found_npc != -1) {
            Mediator::get_instance()->maps_data_npc_list[found_npc].id_npc = -1;
        } else if (Mediator::get_instance()->editTool == EDITMODE_NORMAL && found_npc == -1 && Mediator::get_instance()->selectedNPC != -1) {
            CURRENT_FILE_FORMAT::file_map_npc_v2 new_npc;
            new_npc.id_npc = Mediator::get_instance()->selectedNPC;
            new_npc.start_point.x = editor_selectedTileX;
            new_npc.start_point.y = editor_selectedTileY;
            new_npc.direction = Mediator::get_instance()->npc_direction;
            new_npc.stage_id = Mediator::get_instance()->currentStage;
            new_npc.map_id = Mediator::get_instance()->currentMap;
            new_npc.difficulty_level = Mediator::get_instance()->currentDifficulty;
            new_npc.difficulty_mode = Mediator::get_instance()->currentDifficultyMode;
            Mediator::get_instance()->maps_data_npc_list.push_back(new_npc);
            repaint();
        } else if (Mediator::get_instance()->editTool == EDITMODE_NORMAL && found_npc != -1 && Mediator::get_instance()->selectedNPC != -1) {
			printf(">> EditorArea::mousePressEvent - Adding NPC - place already taken\n");
		}



    } else if (Mediator::get_instance()->editMode == EDITMODE_OBJECT) {
		int found_object = -1;

		// search if there is an existing object in ths position, and if yes, remove it
        for (int m=0; m<Mediator::get_instance()->maps_data_object_list.size(); m++) {
            if (Mediator::get_instance()->maps_data_object_list[m].stage_id != Mediator::get_instance()->currentStage || Mediator::get_instance()->maps_data_object_list[m].map_id != Mediator::get_instance()->currentMap) {
                continue; // only show enemies from current stage/map
            }

            if (Mediator::get_instance()->maps_data_object_list[m].id_object != -1 && Mediator::get_instance()->maps_data_object_list[m].start_point.x == editor_selectedTileX && Mediator::get_instance()->maps_data_object_list[m].start_point.y == editor_selectedTileY) {
                found_object = m;
				break;
			}
		}
		// clean old-format trash
        for (int m=0; m<Mediator::get_instance()->maps_data_object_list.size(); m++) {
            if (Mediator::get_instance()->maps_data_object_list[m].stage_id != Mediator::get_instance()->currentStage || Mediator::get_instance()->maps_data_object_list[m].map_id != Mediator::get_instance()->currentMap) {
                continue; // only show enemies from current stage/map
            }

            if (Mediator::get_instance()->maps_data_object_list[m].start_point.x == -1 && Mediator::get_instance()->maps_data_object_list[m].start_point.y == -1) {
                Mediator::get_instance()->maps_data_object_list[m].id_object = -1;
			}
		}

        if (Mediator::get_instance()->editTool == EDITMODE_ERASER && found_object != -1) {
            Mediator::get_instance()->maps_data_object_list.erase(Mediator::get_instance()->maps_data_object_list.begin()+found_object);
        } else if (Mediator::get_instance()->editTool == EDITMODE_NORMAL && found_object == -1 && Mediator::get_instance()->selectedNPC != -1) {
                CURRENT_FILE_FORMAT::file_map_object_v2 new_obj;
                new_obj.id_object = Mediator::get_instance()->selectedNPC;
                new_obj.start_point.x = editor_selectedTileX;
                new_obj.start_point.y = editor_selectedTileY;
                new_obj.direction = Mediator::get_instance()->object_direction;
                new_obj.stage_id = Mediator::get_instance()->currentStage;
                new_obj.map_id = Mediator::get_instance()->currentMap;
                new_obj.difficulty_level = Mediator::get_instance()->currentDifficulty;
                new_obj.difficulty_mode = Mediator::get_instance()->currentDifficultyMode;
                editor_selected_object_pos_map = Mediator::get_instance()->currentMap;
                // se item é teleportador, deve entrar no modo de colocar link de object
                int obj_type = Mediator::get_instance()->object_list.at(Mediator::get_instance()->selectedNPC).type;
                if (obj_type == OBJ_BOSS_TELEPORTER || obj_type == OBJ_FINAL_BOSS_TELEPORTER || obj_type == OBJ_PLATFORM_TELEPORTER || obj_type == OBJ_STAGE_BOSS_TELEPORTER) {
                    editor_selected_object_pos = Mediator::get_instance()->maps_data_object_list.size();
                    Mediator::get_instance()->editTool = EDITMODE_OBJECT_LINK_PLACING;
                    QApplication::setOverrideCursor(Qt::CrossCursor);
                    // @TODO - desabilita todos os modos, não pode sair no meio de object-link-placing
                    // @TODO - desabilitar mudar de estágio também (e isso tem que ser feito no link tb)
                } else {
                    new_obj.map_dest = -1;
                }
                Mediator::get_instance()->maps_data_object_list.push_back(new_obj);
                repaint();
        } else if (Mediator::get_instance()->editTool == EDITMODE_OBJECT_LINK_PLACING) {
            Mediator::get_instance()->maps_data_object_list[editor_selected_object_pos].link_dest.x = editor_selectedTileX;
            Mediator::get_instance()->maps_data_object_list[editor_selected_object_pos].link_dest.y = editor_selectedTileY;
            Mediator::get_instance()->maps_data_object_list[editor_selected_object_pos].map_dest = Mediator::get_instance()->currentMap;
            Mediator::get_instance()->editTool = EDITMODE_NORMAL;
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        }


    } else if (Mediator::get_instance()->editMode == EDITMODE_SELECT) {
        if (selection_started == false) {
            selection_started = true;
            selection_start_x = editor_selectedTileX;
            selection_start_y = editor_selectedTileY;
        }


    } else if (Mediator::get_instance()->editMode == EDITMODE_PASTE) {
        if (selection_matrix.size() > 0) {
            for (int i=0; i<selection_matrix.size(); i++) {
                for (int j=0; j<selection_matrix.at(i).size(); j++) {
                    st_tile_point tile_point = selection_matrix.at(i).at(j);
                    int x = editor_selectedTileX+i;
                    int y = editor_selectedTileY+j;
                    Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[x][y].tile1 = tile_point.tile1;
                    Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[x][y].tile3 = tile_point.tile3;
                    Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[x][y].locked = tile_point.locked;

                    //Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[x][y].locked
                }
            }
            repaint();
        }
    }
    temp = 1;
    repaint();
}


void EditorArea::mouseReleaseEvent(QMouseEvent *event) {
	QPoint pnt = event->pos();
    editor_selectedTileX = pnt.x()/(16*Mediator::get_instance()->zoom) + 1;
    editor_selectedTileY = pnt.y()/(16*Mediator::get_instance()->zoom) + 1;



    if (Mediator::get_instance()->editMode == EDITMODE_SELECT) {
        // copies points in the selection to the selection matrix


        selection_started = false;
        if (selection_start_x == editor_selectedTileX || selection_start_y == editor_selectedTileY) {
            mouse_released = true;
            return;
        }
        int start_x = selection_start_x;
        int end_x = editor_selectedTileX;
        if (selection_start_x > editor_selectedTileX) {
            start_x = editor_selectedTileX;
            end_x = selection_start_x;
        }
        int start_y = selection_start_y;
        int end_y = editor_selectedTileY;
        if (selection_start_y > editor_selectedTileY) {
            start_y = editor_selectedTileY;
            end_y = selection_start_y;
        }
        selection_matrix.clear();

        for (int i=start_x; i<end_x; i++) {
            std::vector<st_tile_point> temp;
            for (int j=start_y; j<end_y; j++) {
                temp.push_back(st_tile_point(
                     Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1,
                     Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3,
                     Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked
                   ));
            }
            selection_matrix.push_back(temp);
        }
    } else if (Mediator::get_instance()->editTool == EDITMODE_LINK && tempX != -1) {// && Mediator::get_instance()->link_type != LINK_FINAL_BOSS_ROOM) {
		// TODO: add link
        Mediator::get_instance()->editTool = EDITMODE_LINK_DEST;
        QApplication::setOverrideCursor(Qt::CrossCursor);

        link_map_origin = Mediator::get_instance()->currentMap;
		link_pos_x = tempX;
		link_pos_y = tempY;


		int tempSize = abs(editor_selectedTileX - tempX);
		if (tempSize < 1 || tempSize > 20) {
			link_size = 1;
		} else {
            link_size = abs(editor_selectedTileX - tempX);
		}
        //printf(">>>>>>>> added link in map: %d, pos: (%d, %d) with size: %d <<<<<<<<<<\n", link_map_origin, link_pos_x, link_pos_y, link_size);

		tempX = -1;
		tempY = -1;
		repaint();

    }
	mouse_released = true;
}

void EditorArea::fill_area() {
	int i, j;
	int filled1=0;
	int filled2=0;
	int originalX, originalY;
    short int searchX, searchY;

    if (Mediator::get_instance()->layerLevel == 1) {
        originalX = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.x;
        originalY = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.y;
    } else if (Mediator::get_instance()->layerLevel > 1) {
        originalX = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.x;
        originalY = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.y;
	}
	// área a esquerda
	for (i=editor_selectedTileX; i>=0; i--) {
		// área acima
		filled1 = 0;
		filled2 = 0;
		for (j=editor_selectedTileY; j>=0; j--) {
            if (Mediator::get_instance()->layerLevel == 1) {
                searchX = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x;
                searchY = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y;
            } else if (Mediator::get_instance()->layerLevel > 1) {
                searchX = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x;
                searchY = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.y;
			}
			//printf("DEBUG.EditorArea::i: %d, j: %d, fill_area - tile.x: %d, tile;y: %d, originalX: %d, originalY: %d\n", i, j, map.tiles[i][j].tile1.x, map.tiles[i][j].tile1.y, originalX, originalY);
            if (searchX == originalX && searchY == originalY) {
                searchX = Mediator::get_instance()->getPalleteX();
                searchY = Mediator::get_instance()->getPalleteY();
				filled1 = 1;
			} else {
				printf("1.not filling\n");
				break;
			}
		}
		// área abaixo
		for (j=editor_selectedTileY+1; j<MAP_H; j++) {
            if (Mediator::get_instance()->layerLevel == 1) {
                searchX = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x;
                searchY = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y;
            } else if (Mediator::get_instance()->layerLevel > 1) {
                searchX = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x;
                searchY = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.y;
			}
            if (searchX == originalX && searchY == originalY) {
                searchX = Mediator::get_instance()->getPalleteX();
                searchY = Mediator::get_instance()->getPalleteY();
				filled2 = 1;
			} else {
				printf("2.not filling\n");
				break;
			}
		}
		if (filled1 == 0 && filled2 == 0) {
			break;
		}

	}
	// area a direita
	for (i=editor_selectedTileX+1; i<MAP_W; i++) {
		filled1 = 0;
		filled2 = 0;
		for (j=editor_selectedTileY; j>=0; j--) {
			//printf("DEBUG.EditorArea::i: %d, j: %d, fill_area - tile.x: %d, tile;y: %d, originalX: %d, originalY: %d\n", i, j, map.tiles[i][j].tile1.x, map.tiles[i][j].tile1.y, originalX, originalY);
            if (Mediator::get_instance()->layerLevel == 1) {
                searchX = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x;
                searchY = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y;
            } else if (Mediator::get_instance()->layerLevel > 1) {
                searchX = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x;
                searchY = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.y;
			}
            if (searchX == originalX && searchY == originalY) {
                searchX = Mediator::get_instance()->getPalleteX();
                searchY = Mediator::get_instance()->getPalleteY();
				filled1 = 1;
			} else {
				printf("3.not filling\n");
				break;
			}
		}
		// área abaixo
		for (j=editor_selectedTileY+1; j<MAP_H; j++) {
            if (Mediator::get_instance()->layerLevel == 1) {
                searchX = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x;
                searchY = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y;
            } else if (Mediator::get_instance()->layerLevel > 1) {
                searchX = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x;
                searchY = Mediator::get_instance()->maps_data_v2[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.y;
			}
            if (searchX == originalX && searchY == originalY) {
                searchX = Mediator::get_instance()->getPalleteX();
                searchY = Mediator::get_instance()->getPalleteY();
				filled2 = 1;
			} else {
				printf("4.not filling\n");
				break;
			}
		}
		if (filled1 == 0 || filled2 == 0) {
			break;
		}
	}
}







