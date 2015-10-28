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
    my_pallete = NULL;
    editor_selected_object_pos = 0;
    editor_selected_object_pos_map = 0;
    this->show();
}

void EditorArea::changeTile() {
	printf("DEBUG.changeTile in editor area\n");
	temp = 1;
	repaint();
}

void EditorArea::setInfoPalette(EditorTilePallete *temp_pallete) {
   if (temp_pallete != NULL) {
      printf("DEBUG.EditorArea::setPallete - got pallete\n");
   } else {
      printf("DEBUG.EditorArea::setPallete - null pallete\n");
   }
   my_pallete = temp_pallete;
}

void EditorArea::paintEvent(QPaintEvent *) {

    if (Mediator::get_instance()->currentStage < 0) {
        return;
    }

    int i, j, pos;
    QPainter painter(this);
    QLineF line;
    QString filename;

    filename = QString("");
    filename.append(my_pallete->getPallete());
    if (filename.length() <= 0) {
        return;
    }
    //std::cout << "EditorArea::paintEvent - filename: " << filename.toStdString() << std::endl;
    QPixmap image(filename);
    if (image.isNull()) {
        std::cout << "ERROR: EditorArea::paintEvent - Could not load palette image file [" << filename.toStdString() << "]." << std::endl;
        return;
    }

	// draw background-color
    if (Mediator::get_instance()->show_background_color == true) {
        //painter.setPen(QColor(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.r, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.g, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.b, 255));
        painter.fillRect(QRectF(0.0, 0.0, MAP_W*TILESIZE*Mediator::get_instance()->zoom, MAP_H*TILESIZE*Mediator::get_instance()->zoom), QColor(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.r, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.g, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].background_color.b, 255));
	}
	// draw background1
    std::string bg1_filename(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[0].filename);
    if (Mediator::get_instance()->show_bg1 == true && bg1_filename.length() > 0) {
        filename.clear();
        filename.append(QString(QString(FILEPATH.c_str())+"/images/map_backgrounds/"+QString(bg1_filename.c_str())));
        // check that filename contains ".png"
        if (filename.indexOf(".png") != -1) {
            QPixmap bg1_image(filename);
            if (!bg1_image.isNull()) {
                int max_repeat = ((MAP_W*16)/bg1_image.width())*Mediator::get_instance()->zoom+1;
                //std::cout << "bg1_image.width(): " << bg1_image.width() << ", max_repeat: " << max_repeat << std::endl;
                for (int k=0; k<max_repeat; k++) {
                    QRectF pos_source(QPoint(0, 0), QSize(bg1_image.width(), bg1_image.height()));
                    QRectF pos_dest(QPoint(k*bg1_image.width()*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[0].adjust_y*Mediator::get_instance()->zoom), QSize(bg1_image.width()*Mediator::get_instance()->zoom, bg1_image.height()*Mediator::get_instance()->zoom));
                    painter.drawPixmap(pos_dest, bg1_image, pos_source);
                }
            }
        }
	}
	// draw background2
    std::string bg2_filename(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[1].filename);
    if (Mediator::get_instance()->show_bg2 == true && bg2_filename.length() > 0) {
        filename.clear();
        filename.append(QString(QString(FILEPATH.c_str())+"/images/map_backgrounds/"+QString(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[1].filename)));
        if (filename.indexOf(".png") != -1) {
            QPixmap bg2_image(filename);
            if (!bg2_image.isNull()) {
                for (int k=0; k<((MAP_W*16)/bg2_image.width()*Mediator::get_instance()->zoom)+1; k++) {
                    QRectF pos_source(QPoint(0, 0), QSize(bg2_image.width(), bg2_image.height()));
                    QRectF pos_dest(QPoint(k*bg2_image.width()*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].backgrounds[1].adjust_y*Mediator::get_instance()->zoom), QSize(bg2_image.width()*Mediator::get_instance()->zoom, bg2_image.height()*Mediator::get_instance()->zoom));
                    painter.drawPixmap(pos_dest, bg2_image, pos_source);
                }
            }
        }
	}


    // draw tiles
    for (i=0; i<MAP_W; i++) {
        for (j=0; j<MAP_H; j++) {
            // level one
            if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x >= 0 && Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y >= 0) {
                QRectF target(QPoint(i*16*Mediator::get_instance()->zoom, j*16*Mediator::get_instance()->zoom), QSize(16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom));
                QRectF source(QPoint((Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x*16), (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y*16)), QSize(16, 16));
                painter.drawPixmap(target, image, source);
            // animated tiles
            } else if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x < -1 && Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y == 0) {
                int anim_tile_id = (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x*-1) - 2;

                std::cout << "********** (READ) anim-tile-id: " << anim_tile_id << std::endl;

                QString anim_tile_filename = QString(FILEPATH.c_str()) + QString("/images/tilesets/anim/") + QString(Mediator::get_instance()->game_data.anim_tiles[anim_tile_id].filename);

                QPixmap anim_image(anim_tile_filename);

                if (anim_image.isNull() == false) {
                    QRectF target(QPoint(i*TILESIZE*Mediator::get_instance()->zoom, j*TILESIZE*Mediator::get_instance()->zoom), QSize(TILESIZE*Mediator::get_instance()->zoom, TILESIZE*Mediator::get_instance()->zoom));
                    QRectF source(QPoint(0, 0), QSize(TILESIZE, TILESIZE));

                    painter.drawPixmap(target, anim_image, source);
                    // draw an orange border border to indicate anim tile
                    QPen pen(QColor(0, 200, 0), 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
                    painter.setPen(pen);

                    int anim_tile_x = i * TILESIZE * Mediator::get_instance()->zoom; // minus tilesize is because width starts in 1, not zero
                    int anim_tile_y = j * TILESIZE *Mediator::get_instance()->zoom;
                    painter.drawLine(anim_tile_x, anim_tile_y, anim_tile_x+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_y);
                    painter.drawLine(anim_tile_x, anim_tile_y, anim_tile_x, anim_tile_y+(TILESIZE*Mediator::get_instance()->zoom));
                    painter.drawLine(anim_tile_x, anim_tile_y+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_x+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_y+(TILESIZE*Mediator::get_instance()->zoom));
                    painter.drawLine(anim_tile_x+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_y, anim_tile_x+(TILESIZE*Mediator::get_instance()->zoom), anim_tile_y+(TILESIZE*Mediator::get_instance()->zoom));
                } else {
                    std::cout << ">>>>>>>> anim-file '" << anim_tile_filename.toStdString() << "' not found." << std::endl;
                }
            }
            // level 3
            if (Mediator::get_instance()->layerLevel == 3) {
                    if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x != -1 && Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.y != -1) {
                        QRectF target(QPoint(i*16*Mediator::get_instance()->zoom, j*16*Mediator::get_instance()->zoom), QSize(16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom));
                        QRectF source(QPoint((Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x*16), (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.y*16)), QSize(16, 16));
                        painter.drawPixmap(target, image, source);
                    }
            }
			// locked areas, stairs, doors, etc
            if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked != 0 && Mediator::get_instance()->editTool == EDITMODE_LOCK) {
                    // transparent rectangle
					//painter.setBrush(QColor(255, 0, 0, 30));
					//painter.drawRect(i*16, j*16, 16, 16);
                    // red border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(255, 0, 0, 255));
                    painter.drawRect(i*16*Mediator::get_instance()->zoom, j*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom);
					// terrain type icon
					QString terrainIcon;
					QResource::registerResource("resources/icons/icons.qrc");
                    if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 1) {
						terrainIcon = QString::fromUtf8(":/toolbar_icons/Lock"); // solid
                    } else if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 2) {
						terrainIcon = QString(":/toolbar_icons/stairs.png"); // stairs
                    } else if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 3) {
						terrainIcon = QString(":/toolbar_icons/object-order-lower.png"); // door
                    } else if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 4) {
						terrainIcon = QString(":/toolbar_icons/edit-delete.png"); // spikes
                    } else if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 5) {
						terrainIcon = QString(":/toolbar_icons/flag-blue.png"); // water
                    } else if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 6) {
						terrainIcon = QString(":/toolbar_icons/flag-green.png"); // ice
                    } else if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 7) {
						terrainIcon = QString(":/toolbar_icons/arrow-left.png"); // move left
                    } else if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 8) {
						terrainIcon = QString(":/toolbar_icons/arrow-right.png"); // move right
                    } else if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 9) {
						terrainIcon = QString(":/toolbar_icons/arrow-down.png"); // move right
                    } else if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 10) {
						terrainIcon = QString(":/toolbar_icons/system-switch-user.png"); // move right
                    } else if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].locked == 11) {
						terrainIcon = QString(":/toolbar_icons/Save"); // move right
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



    QPen pen(QColor(120, 120, 120), 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    for (i=1; i<MAP_W; i++) {
        pos = i*16*Mediator::get_instance()->zoom-1;
        //QLineF line(0, 800, 16, 800);
        // linhas horizontais
        line = QLineF(pos, 0, pos, MAP_H*16*Mediator::get_instance()->zoom-1);
        painter.drawLine(line);
    }
    for (i=1; i<MAP_H; i++) {
        pos = i*16*Mediator::get_instance()->zoom-1;
        //QLineF line(0, 800, 16, 800);
        // linhas verticais
        line = QLineF(0, pos, MAP_W*16*Mediator::get_instance()->zoom-1, pos);
        painter.drawLine(line);
    }



    if (Mediator::get_instance()->show_teleporters_flag == true) {
        // draw links
        //printf("editoMode: %d, EDITMODE_NORMAL: %d, editTool: %d, EDITMODE_LINK_DEST: %d\n", Mediator::get_instance()->editMode, EDITMODE_NORMAL, Mediator::get_instance()->editTool, EDITMODE_LINK_DEST);
        if (Mediator::get_instance()->editMode == EDITMODE_LINK && (Mediator::get_instance()->editTool == EDITMODE_LINK_DEST || Mediator::get_instance()->editTool == EDITMODE_LINK)) {
            for (int i=0; i<STAGE_MAX_LINKS; i++) {
                if (Mediator::get_instance()->currentMap == Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_origin) {
                    // transparent blue rectangle
                    if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].type == LINK_FINAL_BOSS_ROOM) {
                        painter.setBrush(QColor(255, 0, 0, 180));
                    } else if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].type == LINK_TELEPORTER) {
                        painter.setBrush(QColor(0, 255, 0, 180));
                    } else if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].type == LINK_BOSS_TELEPORTER) {
                        painter.setBrush(QColor(255, 100, 0, 180));
                    } else {
                        painter.setBrush(QColor(0, 0, 255, 180));
                    }
                    painter.drawRect(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.y *16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom*Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].size, 16*Mediator::get_instance()->zoom);
                    // blue border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(0, 0, 255, 255));
                    painter.drawRect(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.y*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom*Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].size, 16*Mediator::get_instance()->zoom);
                    painter.setPen(QColor(255, 255, 255, 255));
                    painter.drawText(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.x*16*Mediator::get_instance()->zoom, (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_origin.y+1)*16*Mediator::get_instance()->zoom, QString::number(i));
                } else if (Mediator::get_instance()->currentMap == Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_destiny) {
                    // transparent cyan rectangle
                    if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].type == LINK_FINAL_BOSS_ROOM) {
                        painter.setBrush(QColor(160, 60, 60, 180));
                    } else if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].type == LINK_TELEPORTER) {
                        painter.setBrush(QColor(60, 160, 60, 180));
                    } else if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].type == LINK_BOSS_TELEPORTER) {
                        painter.setBrush(QColor(160, 60, 60, 180));
                    } else {
                        painter.setBrush(QColor(0, 255, 255, 180));
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



    if (Mediator::get_instance()->show_npcs_flag == true) {
        /// draw NPCs
        for (int i=0; i<FS_MAX_MAP_NPCS; i++) {

            //std::cout << "EditorArea::paintEvent #5.0.A [" << i << "]" << std::endl;
            int npc_id = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc;
            if (npc_id >= FS_GAME_MAX_NPCS || npc_id < 0) {
                //std::cout << "EditorArea::paintEvent #5.0.B - stage[" << Mediator::get_instance()->currentStage << "], map[" << Mediator::get_instance()->currentMap << "], i[" << i << "], id_npc[" << npc_id << "] BAD" << std::endl;
                Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc = -1;
                continue;
    //        } else {
    //            std::cout << "EditorArea::paintEvent #5.0.B - i[" << i << "], id_npc[" << npc_id << "] GOOD" << std::endl;
            }

            //std::cout << "EditorArea::paintEvent #5.0.C graphic_filename[" << game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].graphic_filename << "]" << std::endl;
            std::string filename = FILEPATH + "/images/sprites/enemies/" + Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].graphic_filename;
            if (filename.length() <= 0) {
                continue;
            }


            QPixmap temp_image(filename.c_str());
            if (!temp_image.isNull()) {

                int total_w = Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].frame_size.width*Mediator::get_instance()->zoom;
                int total_h = Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].frame_size.height*Mediator::get_instance()->zoom;
                int sprite_adjust_x = Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].sprites_pos_bg.x;
                int sprite_adjust_y = Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].sprites_pos_bg.y;


                std::string npc_bg_file(Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].bg_graphic_filename);
                if (npc_bg_file.length() > 0) {
                    std::string _bg_graphic_filename = FILEPATH + "/images/sprites/backgrounds/" + npc_bg_file;
                    QPixmap bg_image(_bg_graphic_filename.c_str());

                    // calculate total image size of background exists
                    if (!bg_image.isNull()) {
                        total_w = bg_image.width();
                        total_h = bg_image.height();
                    }
                    if (!bg_image.isNull()) {
                        QRectF bg_target(QPoint(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.y*16*Mediator::get_instance()->zoom), QSize(total_w, total_h));
                        QRectF bg_source(QRectF(QPoint(0, 0), QSize(bg_image.width(), bg_image.height())));
                        painter.drawPixmap(bg_target, bg_image, bg_source);
                    }
                }

                QRectF target(QPoint((Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.x*16+sprite_adjust_x)*Mediator::get_instance()->zoom, (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.y*16+sprite_adjust_y)*Mediator::get_instance()->zoom), QSize(total_w, total_h));
                QRectF source;
                if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].direction != ANIM_DIRECTION_RIGHT || temp_image.height() <= Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].frame_size.height) {
                    source = QRectF(QPoint(0, 0), QSize(Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].frame_size.width, Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].frame_size.height));
                } else {
                    source = QRectF(QPoint(0, Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].frame_size.height), QSize(Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].frame_size.width, Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].frame_size.height));
                }
                if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.id_npc == Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc) {
                    // transparent green rectangle
                    painter.setBrush(QColor(0, 255, 0, 180));
                    painter.drawRect(target);
                } else if (Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].is_boss) {
                    // transparent orange rectangle
                    painter.setBrush(QColor(255, 128, 35, 200));
                    painter.drawRect(target);
                } else if (Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].is_sub_boss) {
                    // transparent bright-orange rectangle
                    painter.setBrush(QColor(241, 188, 87, 200));
                    painter.drawRect(target);
                }
                painter.drawPixmap(target, temp_image, source);
            }
        }
    }

    if (Mediator::get_instance()->show_objects_flag == true) {
        /// draw objects
        //std::cout << "################### START LOOP" << std::endl;
        for (int i=0; i<FS_MAX_MAP_OBJECTS; i++) {
            int obj_id = (int)Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].id_object;
            if (obj_id != -1) {
                //if (obj_id == 20) { std::cout << "************************** paintEvent - draw_objects[" << i << "].id: " << obj_id << std::endl; }
                std::string filename = FILEPATH + "/images/sprites/objects/" + Mediator::get_instance()->game_data.objects[obj_id].graphic_filename;
                QPixmap temp_image(filename.c_str());
                if (temp_image.isNull()) {
                    std::cout << "****************** Could not load file '" << filename.c_str() << "'" << std::endl;
                    painter.setBrush(QColor(255, 255, 255, 180));
                    painter.drawRect(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.y*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom);
                } else {
                    int obj_type = Mediator::get_instance()->game_data.objects[obj_id].type;
                    int obj_direction = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].direction;
                    if (obj_type == OBJ_RAY_HORIZONTAL) {
                        int graphic_pos_x = 0;
                        int graphic_pos_y = Mediator::get_instance()->game_data.objects[obj_id].size.height;
                        if (obj_direction == ANIM_DIRECTION_LEFT) {
                            graphic_pos_x = Mediator::get_instance()->game_data.objects[obj_id].size.width - TILESIZE;
                            graphic_pos_y = 0;
                        }
                        //std::cout << "OBJ_RAY_HORIZONTAL - obj_direction: " << obj_direction << ", graphic_pos_x: " << graphic_pos_x << ", graphic_pos_y: " << graphic_pos_y << std::endl;
                        QRectF target(QPoint(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.x*TILESIZE*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.y*TILESIZE*Mediator::get_instance()->zoom), QSize(TILESIZE*Mediator::get_instance()->zoom, Mediator::get_instance()->game_data.objects[obj_id].size.height*Mediator::get_instance()->zoom));
                        QRectF source(QPoint(graphic_pos_x, graphic_pos_y), QSize(TILESIZE, Mediator::get_instance()->game_data.objects[obj_id].size.height));
                        painter.drawPixmap(target, temp_image, source);
                    } else if (obj_type == OBJ_RAY_VERTICAL) {
                        int graphic_pos_y = Mediator::get_instance()->game_data.objects[obj_id].size.height - TILESIZE;
                        //std::cout << "OBJ_RAY_HORIZONTAL - graphic_pos_y: " << graphic_pos_y << std::endl;
                        QRectF target(QPoint(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.y*16*Mediator::get_instance()->zoom), QSize(Mediator::get_instance()->game_data.objects[obj_id].size.width*Mediator::get_instance()->zoom, Mediator::get_instance()->game_data.objects[obj_id].size.height*Mediator::get_instance()->zoom));
                        QRectF source(QPoint(0, graphic_pos_y), QSize(Mediator::get_instance()->game_data.objects[obj_id].size.width, Mediator::get_instance()->game_data.objects[obj_id].size.height));
                        painter.drawPixmap(target, temp_image, source);
                    } else if (obj_type == OBJ_DEATHRAY_HORIZONTAL && obj_direction == ANIM_DIRECTION_LEFT) {
                        QRectF target(QPoint(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.x*TILESIZE*Mediator::get_instance()->zoom - (Mediator::get_instance()->game_data.objects[obj_id].size.width-TILESIZE)*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.y*16*Mediator::get_instance()->zoom), QSize(Mediator::get_instance()->game_data.objects[obj_id].size.width*Mediator::get_instance()->zoom, Mediator::get_instance()->game_data.objects[obj_id].size.height*Mediator::get_instance()->zoom));
                        QRectF source(QPoint(0, 0), QSize(Mediator::get_instance()->game_data.objects[obj_id].size.width, Mediator::get_instance()->game_data.objects[obj_id].size.height));
                        painter.drawPixmap(target, temp_image, source);
                    } else {
                        QRectF target(QPoint(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.y*16*Mediator::get_instance()->zoom), QSize(Mediator::get_instance()->game_data.objects[obj_id].size.width*Mediator::get_instance()->zoom, Mediator::get_instance()->game_data.objects[obj_id].size.height*Mediator::get_instance()->zoom));
                        QRectF source;
                        if (obj_direction == ANIM_DIRECTION_RIGHT && temp_image.height() >= Mediator::get_instance()->game_data.objects[obj_id].size.height) {
                            source = QRectF(QPoint(0, Mediator::get_instance()->game_data.objects[obj_id].size.height), QSize(Mediator::get_instance()->game_data.objects[obj_id].size.width, Mediator::get_instance()->game_data.objects[obj_id].size.height));
                        } else {
                            source = QRectF(QPoint(0, 0), QSize(Mediator::get_instance()->game_data.objects[obj_id].size.width, Mediator::get_instance()->game_data.objects[obj_id].size.height));
                        }
                        painter.drawPixmap(target, temp_image, source);
                    }
                }
                // draw object-teleporter origin
                if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].map_dest != -1) {
                    int obj_w = Mediator::get_instance()->game_data.objects[obj_id].size.width;
                    int obj_h = Mediator::get_instance()->game_data.objects[obj_id].size.height;
                    std::cout << "OBJ.w: " << obj_w << ", obj_h: " << obj_h << std::endl;
                    int dest_x = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.x*TILESIZE*Mediator::get_instance()->zoom + (obj_w/2)*Mediator::get_instance()->zoom - TILESIZE/2;
                    int dest_y = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.y*TILESIZE*Mediator::get_instance()->zoom + (obj_h/2)*Mediator::get_instance()->zoom - TILESIZE/2;
                    std::cout << "DRAW OBJECT TELEPORTER ORIGIN[" << i << "] - map: " << (int)Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].map_dest  << ", x: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.x << ", y: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.y << std::endl;
                    if (Mediator::get_instance()->game_data.objects[obj_id].type == OBJ_FINAL_BOSS_TELEPORTER) {
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

        for (int k=0; k<FS_STAGE_MAX_MAPS; k++) {
            for (int i=0; i<FS_MAX_MAP_OBJECTS; i++) {
                // draw teleport destiny links
                //std::cout << "OBJ[" << i << "].map_dest: " << (int)Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][k].map_objects[i].map_dest << ", currentMap: " << k << std::endl;
                if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][k].map_objects[i].map_dest == Mediator::get_instance()->currentMap) {
                    int obj_id = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][k].map_objects[i].id_object;
                    std::cout << "## EDITORAREA::paintEvent - teleporter_obj - x: " << (int)Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][k].map_objects[i].link_dest.x << ", y: " << (int)Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][k].map_objects[i].link_dest.y << std::endl;
                    if (Mediator::get_instance()->game_data.objects[obj_id].type == OBJ_FINAL_BOSS_TELEPORTER) {
                        painter.setBrush(QColor(160, 60, 60, 180));
                    } else {
                        painter.setBrush(QColor(60, 160, 60, 180));
                    }
                    painter.drawEllipse(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][k].map_objects[i].link_dest.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][k].map_objects[i].link_dest.y*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom);
                    // cyan border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(0, 255, 255, 255));
                    painter.drawEllipse(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][k].map_objects[i].link_dest.x*16*Mediator::get_instance()->zoom, Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][k].map_objects[i].link_dest.y*16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom, 16*Mediator::get_instance()->zoom);
                    painter.setPen(QColor(0, 0, 0, 255));
                    painter.drawText(Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][k].map_objects[i].link_dest.x*16*Mediator::get_instance()->zoom + 3*Mediator::get_instance()->zoom, (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][k].map_objects[i].link_dest.y+1)*16*Mediator::get_instance()->zoom -2*Mediator::get_instance()->zoom, QString::number(i));
                }
            }
        }
    }


    QSize resizeMe(MAP_W*16*Mediator::get_instance()->zoom, MAP_H*16*Mediator::get_instance()->zoom);
    this->resize(resizeMe);
    myParent->adjustSize();

}

void EditorArea::mouseMoveEvent(QMouseEvent *event) {
	QPoint pnt = event->pos();
        if (editor_selectedTileX != pnt.x()/(16*Mediator::get_instance()->zoom) || editor_selectedTileY != pnt.y()/(16*Mediator::get_instance()->zoom)) {
		mousePressEvent(event);
	}
}

void EditorArea::mousePressEvent(QMouseEvent *event) {
    if (mouse_released == false && (Mediator::get_instance()->editTool == EDITMODE_LINK || Mediator::get_instance()->editTool == EDITMODE_LINK_DEST || Mediator::get_instance()->editMode == EDITMODE_NPC || Mediator::get_instance()->editMode == EDITMODE_OBJECT || Mediator::get_instance()->editMode == EDITMODE_SET_BOSS || Mediator::get_instance()->editMode == EDITMODE_SET_SUBBOSS)) {
		return;
	}

    QPoint pnt = event->pos();
    editor_selectedTileX = pnt.x()/(16*Mediator::get_instance()->zoom);
    editor_selectedTileY = pnt.y()/(16*Mediator::get_instance()->zoom);

    if (Mediator::get_instance()->editMode == EDITMODE_NORMAL) {
        if (Mediator::get_instance()->editTool == EDITMODE_NORMAL) {
            if (Mediator::get_instance()->layerLevel == 1) {
                Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.x = Mediator::get_instance()->getPalleteX();
                Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.y = Mediator::get_instance()->getPalleteY();
            } else if (Mediator::get_instance()->layerLevel == 3) {
				printf(">> EditorArea::mousePressEvent - placing tile 3\n");
                Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.x = Mediator::get_instance()->getPalleteX();
                Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.y = Mediator::get_instance()->getPalleteY();
			}
        } else if (Mediator::get_instance()->editTool == EDITMODE_ERASER) {
            if (Mediator::get_instance()->layerLevel == 1) {
                Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.x = -1;
                Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.y = -1;
            } else if (Mediator::get_instance()->layerLevel == 3) {
                Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.x = -1;
                Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.y = -1;
			}
        } else if (Mediator::get_instance()->editTool == EDITMODE_LOCK) {
            if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].locked == 0) {
                Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].locked = Mediator::get_instance()->terrainType;
			} else {
                Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].locked = 0;
			}

        } else if (Mediator::get_instance()->editTool == EDITMODE_FILL) {
			fill_area();

        }

    } else if (Mediator::get_instance()->editMode == EDITMODE_ANIM_TILE) {
        // we set x as anim_tile.id -2 and y as zero
        int anim_tile_id = (Mediator::get_instance()->selectedAnimTileset*-1) - 2;
        std::cout << "$$$$$$$$$$$$$$$$$ ADD-ANIM-TILE, anim_tile_id: " << anim_tile_id << std::endl;
        Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.x = anim_tile_id;
        Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.y = 0;



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
						std::cout << "######## -> editorArea::mousePress - removed link 1" << std::endl;
						removed_link = true;
						break;
					}
                } else if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_destiny == Mediator::get_instance()->currentMap) {
                    if (Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_destiny.x == editor_selectedTileX && Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].pos_destiny.y == editor_selectedTileY) {
                        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_destiny = -1;
                        Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_origin = -1;
						std::cout << "######## -> editorArea::mousePress - removed link 2" << std::endl;
						removed_link = true;
						break;
					}
				}
			}
			if (removed_link == false) {
				// check if there is a slot free
				int link_n = -1;
				for (int i=0; i<STAGE_MAX_LINKS; i++) {
                    std::cout << ">> DEBUG - links[" << i << "].id_map_destiny: " << Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_destiny << ", id_map_origin: " << Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[i].id_map_origin << std::endl;
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
                if (Mediator::get_instance()->editTool == EDITMODE_LINK && tempX == -1 && Mediator::get_instance()->link_type == LINK_FINAL_BOSS_ROOM) {
                    std::cout << "########### -> editorArea::mousePress - adding LAST BOSS point" << std::endl;
                    Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].id_map_origin = link_map_origin;
                    Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].pos_origin.x = editor_selectedTileX;
                    Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].pos_origin.y = editor_selectedTileY;
                    Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].size = link_size;
                    Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].id_map_destiny = Mediator::get_instance()->currentMap;
                    Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].pos_destiny.x = editor_selectedTileX;
                    Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].pos_destiny.y = editor_selectedTileY;
                    /// @TODO: this must come from editor properties
                    Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].bidirecional = false;
                    Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].is_door = false;
                    Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].links[link_n].type = Mediator::get_instance()->link_type;
                } else {
                    std::cout << "########### -> editorArea::mousePress - adding link ORIGIN at slot[" << link_n << "] - PART 1" << std::endl;
                    tempX = editor_selectedTileX;
                    tempY = editor_selectedTileY;
                }
			}
        } else if (Mediator::get_instance()->editTool == EDITMODE_LINK_DEST) {
			std::cout << "########### -> editorArea::mousePress - adding link - PART 3" << std::endl;
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
			std::cout << "########### -> editorArea::mousePress - adding link DESTINY at slot[" << link_n << "] - PART 3" << std::endl;
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
        std::cout << ">> EditorArea::mousePressEvent - EDITMODE_SET_BOSS" << std::endl;
        // search if there is an existing NPC in ths position, and if yes, set as boss
        for (int i=0; i<MAX_MAP_NPC_N; i++) {
            if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc != -1 && Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.x == editor_selectedTileX && Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.y == editor_selectedTileY) {
                Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.id_npc = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc;
                sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.name, "%s", Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].name);
                break;
            }
        }


    } else if (Mediator::get_instance()->editMode == EDITMODE_SET_SUBBOSS) {
        std::cout << ">> EditorArea::mousePressEvent - EDITMODE_SET_BOSS" << std::endl;
        // search if there is an existing NPC in ths position, and if yes, set as sub-boss
        for (int i=0; i<MAX_MAP_NPC_N; i++) {
            if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc != -1 && Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.x == editor_selectedTileX && Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.y == editor_selectedTileY) {
                std::cout << ">> EditorArea::mousePressEvent - EDITMODE_SET_BOSS - FOUND NPC" << std::endl;
                Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].is_sub_boss = !Mediator::get_instance()->game_data.game_npcs[Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc].is_sub_boss;
                break;
            }
        }



/// @TODO: convert to new file-system ///

    } else if (Mediator::get_instance()->editMode == EDITMODE_NPC) {
		printf(">> EditorArea::mousePressEvent - EDITMODE_NPC\n");

		int found_npc = -1;
		// search if there is an existing NPC in ths position, and if yes, remove it
		for (int i=0; i<MAX_MAP_NPC_N; i++) {
            if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc != -1 && Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.x == editor_selectedTileX && Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.y == editor_selectedTileY) {
				found_npc = i;
				break;
			}
		}
		// clean old-format trash
        for (int i=0; i<FS_MAX_MAP_OBJECTS; i++) {
            if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.x == -1 && Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.y == -1) {
                std::cout << "[OBJECT] reseting free-slot - i: " << i << ", id: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc << ", x: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.x << ", y: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.y << std::endl;
                Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc = -1;
			}
		}


        if (Mediator::get_instance()->editTool == EDITMODE_ERASER && found_npc != -1) {
			std::cout << "remove npc - slot: " << found_npc << std::endl;
            Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[found_npc].id_npc = -1;
        } else if (Mediator::get_instance()->editTool == EDITMODE_NORMAL && found_npc == -1 && Mediator::get_instance()->selectedNPC != -1) {
			// search for free slot
			for (int i=0; i<MAX_MAP_NPC_N; i++) {
                //std::cout << "searching for free-slot - i: " << i << ", id: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc << ", x: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.x << ", y: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.y << std::endl;
                if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc == -1) {

                    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc = Mediator::get_instance()->selectedNPC;

                    std::cout << ">> added npc to map - stage[" << Mediator::get_instance()->currentStage << "], map[" << Mediator::get_instance()->currentMap << "], slot[" << i << "], id[" << Mediator::get_instance()->selectedNPC << "], set_value[" << (int)Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].id_npc << "]" << std::endl;

                    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.x = editor_selectedTileX;
                    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].start_point.y = editor_selectedTileY;
                    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_npcs[i].direction = Mediator::get_instance()->npc_direction;
					repaint();
					return;
				}
			}
			QMessageBox msgBox;
			msgBox.setText("No free slot to place NPC in map.");
			msgBox.exec();
			return;
        } else if (Mediator::get_instance()->editTool == EDITMODE_NORMAL && found_npc != -1 && Mediator::get_instance()->selectedNPC != -1) {
			printf(">> EditorArea::mousePressEvent - Adding NPC - place already taken\n");
		}



    } else if (Mediator::get_instance()->editMode == EDITMODE_OBJECT) {


        std::cout << ">> EditorArea::mousePressEvent - EDITMODE_OBJECT" << std::endl;

		int found_object = -1;
		// search if there is an existing object in ths position, and if yes, remove it
        for (int i=0; i<FS_MAX_MAP_OBJECTS; i++) {
            if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].id_object != -1 && Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.x == editor_selectedTileX && Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.y == editor_selectedTileY) {
				found_object = i;
				break;
			}
		}
		// clean old-format trash
        for (int i=0; i<FS_MAX_MAP_OBJECTS; i++) {
            if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.x == -1 && Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.y == -1) {
                std::cout << "reseting free-slot - i: " << i << ", id: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].id_object << ", x: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.x << ", y: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.y << std::endl;
                Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].id_object = -1;
			}
		}





        if (Mediator::get_instance()->editTool == EDITMODE_ERASER && found_object != -1) {
			std::cout << "remove object - slot: " << found_object << std::endl;
            Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[found_object].id_object = -1;
        } else if (Mediator::get_instance()->editTool == EDITMODE_NORMAL && found_object == -1 && Mediator::get_instance()->selectedNPC != -1) {
			// search for free slot
            for (int i=0; i<FS_MAX_MAP_OBJECTS; i++) {
                std::cout << "[OBJECT] searching for free-slot - i: " << i << ", id: " << (int)Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].id_object << ", x: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.x << ", y: " << Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.y << std::endl;
                if (Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].id_object == -1) {
                    std::cout << "add object - slot: " << i << ", id: " << (int)Mediator::get_instance()->selectedNPC << std::endl;
                    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].id_object = Mediator::get_instance()->selectedNPC;
                    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.x = editor_selectedTileX;
                    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].start_point.y = editor_selectedTileY;
                    Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].direction = Mediator::get_instance()->object_direction;
                    editor_selected_object_pos_map = Mediator::get_instance()->currentMap;
                    // se item é teleportador, deve entrar no modo de colocar link de object
                    if (Mediator::get_instance()->game_data.objects[Mediator::get_instance()->selectedNPC].type == OBJ_BOSS_TELEPORTER || Mediator::get_instance()->game_data.objects[Mediator::get_instance()->selectedNPC].type == OBJ_FINAL_BOSS_TELEPORTER) {
                        editor_selected_object_pos = i;
                        std::cout << "SET editor_selected_object_pos: " << editor_selected_object_pos << std::endl;
                        Mediator::get_instance()->editTool = EDITMODE_OBJECT_LINK_PLACING;
                        QApplication::setOverrideCursor(Qt::CrossCursor);
                        // @TODO - desabilita todos os modos, não pode sair no meio de object-link-placing
                        // @TODO - desabilitar mudar de estágio também (e isso tem que ser feito no link tb)
                    } else {
                        Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].map_objects[i].map_dest = -1;
                    }
					repaint();
					return;
				}
			}
			QMessageBox msgBox;
			msgBox.setText("No free slot to place object in map.");
			msgBox.exec();
			return;
        } else if (Mediator::get_instance()->editTool == EDITMODE_NORMAL && found_object != -1 && Mediator::get_instance()->selectedNPC != -1) {
			printf(">> EditorArea::mousePressEvent - Adding object - place already taken\n");
        } else if (Mediator::get_instance()->editTool == EDITMODE_OBJECT_LINK_PLACING) {
            std::cout << ">> EditorArea::mousePressEvent - EDITMODE_OBJECT_LINK_PLACING" << std::endl;
            std::cout << "USE editor_selected_object_pos_map: " << editor_selected_object_pos_map << ", editor_selected_object_pos: " << editor_selected_object_pos << ", editor_selectedTileX: " << editor_selectedTileX << ", editor_selectedTileY: " << editor_selectedTileY <<    std::endl;
            Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][editor_selected_object_pos_map].map_objects[editor_selected_object_pos].link_dest.x = editor_selectedTileX;
            Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][editor_selected_object_pos_map].map_objects[editor_selected_object_pos].link_dest.y = editor_selectedTileY;
            Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][editor_selected_object_pos_map].map_objects[editor_selected_object_pos].map_dest = Mediator::get_instance()->currentMap;
            std::cout << "[TELEPORTER] SET map: " << (int)Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][editor_selected_object_pos_map].map_objects[editor_selected_object_pos].map_dest << ", x: " << (int)Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][editor_selected_object_pos_map].map_objects[editor_selected_object_pos].link_dest.x << ", y: " << (int)Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][editor_selected_object_pos_map].map_objects[editor_selected_object_pos].link_dest.y << std::endl;
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        } else {
            std::cout << ">> EditorArea::mousePressEvent - EDITMODE UNKNOWN!!!!" << std::endl;
        }







	}
    temp = 1;
    repaint();
}


void EditorArea::mouseReleaseEvent(QMouseEvent *event) {
	QPoint pnt = event->pos();
    editor_selectedTileX = pnt.x()/(16*Mediator::get_instance()->zoom);
    editor_selectedTileY = pnt.y()/(16*Mediator::get_instance()->zoom);

    if (Mediator::get_instance()->editTool == EDITMODE_LINK && tempX != -1) {// && Mediator::get_instance()->link_type != LINK_FINAL_BOSS_ROOM) {
		std::cout << "########### -> editorArea::mousePress - adding link - PART 2" << std::endl;
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
			link_size = abs(editor_selectedTileX - tempX) + 1;
		}
		printf(">>>>>>>> added link in map: %d, pos: (%d, %d) with size: %d <<<<<<<<<<\n", link_map_origin, link_pos_x, link_pos_y, link_size);

		//printf("editorArea::mouseReleaseEvent - tempX: %d, tempY: %d, p.X: %d, p.y: %d, LINK-SIZE: %d\n", tempX, tempY, editor_selectedTileX, editor_selectedTileY, game.map_links[i].link_size);
		tempX = -1;
		tempY = -1;
		repaint();
    //} else {
        //std::cout << "########### -> editorArea::mousePress - ignore mouse release" << std::endl;
        //std::cout << "######### edit_move: " << Mediator::get_instance()->editTool << ", tempX: " << tempX << std::endl;
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
        originalX = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.x;
        originalY = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.y;
    } else if (Mediator::get_instance()->layerLevel == 3) {
        originalX = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.x;
        originalY = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.y;
	}
	// área a esquerda
	for (i=editor_selectedTileX; i>=0; i--) {
		// área acima
		filled1 = 0;
		filled2 = 0;
		for (j=editor_selectedTileY; j>=0; j--) {
            if (Mediator::get_instance()->layerLevel == 1) {
                searchX = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x;
                searchY = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y;
            } else if (Mediator::get_instance()->layerLevel == 3) {
                searchX = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x;
                searchY = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.y;
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
                searchX = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x;
                searchY = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y;
            } else if (Mediator::get_instance()->layerLevel == 3) {
                searchX = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x;
                searchY = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.y;
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
                searchX = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x;
                searchY = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y;
            } else if (Mediator::get_instance()->layerLevel == 3) {
                searchX = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x;
                searchY = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.y;
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
                searchX = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.x;
                searchY = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile1.y;
            } else if (Mediator::get_instance()->layerLevel == 3) {
                searchX = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.x;
                searchY = Mediator::get_instance()->maps_data[Mediator::get_instance()->currentStage][Mediator::get_instance()->currentMap].tiles[i][j].tile3.y;
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







