#include <iostream>
#include <stdio.h>
#include "editorarea.h"
#include <QApplication>
#include <QResource>
#include <QBrush>
#include <QDir>

#include "defines.h"


extern format_v1_0::file_game game;
extern char EDITOR_FILEPATH[512];

#include "../file/format.h"
#include "../file/file_io.h"
extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_stages stage_data;
extern CURRENT_FILE_FORMAT::file_io fio;


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

    if (dataExchanger->currentStage < 0) {
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
	if (dataExchanger->show_background_color == true) {
        //painter.setPen(QColor(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.r, stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.g, stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.b, 255));
        painter.fillRect(QRectF(0.0, 0.0, MAP_W*TILESIZE*dataExchanger->zoom, MAP_H*TILESIZE*dataExchanger->zoom), QColor(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.r, stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.g, stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].background_color.b, 255));
	}
	// draw background1
    std::string bg1_filename(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].filename);
    if (dataExchanger->show_bg1 == true && bg1_filename.length() > 0) {
        filename.clear();
        filename.append(QString(QString(EDITOR_FILEPATH)+"/data/images/map_backgrounds/"+QString(bg1_filename.c_str())));
        // check that filename contains ".png"
        if (filename.indexOf(".png") != -1) {
            QPixmap bg1_image(filename);
            if (!bg1_image.isNull()) {
                int max_repeat = ((MAP_W*16)/bg1_image.width())*dataExchanger->zoom+1;
                //std::cout << "bg1_image.width(): " << bg1_image.width() << ", max_repeat: " << max_repeat << std::endl;
                for (int k=0; k<max_repeat; k++) {
                    QRectF pos_source(QPoint(0, 0), QSize(bg1_image.width(), bg1_image.height()));
                    QRectF pos_dest(QPoint(k*bg1_image.width()*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].adjust_y*dataExchanger->zoom), QSize(bg1_image.width()*dataExchanger->zoom, bg1_image.height()*dataExchanger->zoom));
                    painter.drawPixmap(pos_dest, bg1_image, pos_source);
                }
            }
        }
	}
	// draw background2
    std::string bg2_filename(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].filename);
    if (dataExchanger->show_bg2 == true && bg2_filename.length() > 0) {
        filename.clear();
        filename.append(QString(QString(EDITOR_FILEPATH)+"/data/images/map_backgrounds/"+QString(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].filename)));
        if (filename.indexOf(".png") != -1) {
            QPixmap bg2_image(filename);
            if (!bg2_image.isNull()) {
                for (int k=0; k<((MAP_W*16)/bg2_image.width()*dataExchanger->zoom)+1; k++) {
                    QRectF pos_source(QPoint(0, 0), QSize(bg2_image.width(), bg2_image.height()));
                    QRectF pos_dest(QPoint(k*bg2_image.width()*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[1].adjust_y*dataExchanger->zoom), QSize(bg2_image.width()*dataExchanger->zoom, bg2_image.height()*dataExchanger->zoom));
                    painter.drawPixmap(pos_dest, bg2_image, pos_source);
                }
            }
        }
	}


    // draw tiles
    for (i=0; i<MAP_W; i++) {
        for (j=0; j<MAP_H; j++) {
            // level one
            if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.x != -1 && stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.y != -1) {
                QRectF target(QPoint(i*16*dataExchanger->zoom, j*16*dataExchanger->zoom), QSize(16*dataExchanger->zoom, 16*dataExchanger->zoom));
                QRectF source(QPoint((stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.x*16), (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.y*16)), QSize(16, 16));
                painter.drawPixmap(target, image, source);
            }
            // level 3
            if (dataExchanger->layerLevel == 3) {
                    if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.x != -1 && stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.y != -1) {
                        QRectF target(QPoint(i*16*dataExchanger->zoom, j*16*dataExchanger->zoom), QSize(16*dataExchanger->zoom, 16*dataExchanger->zoom));
                        QRectF source(QPoint((stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.x*16), (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.y*16)), QSize(16, 16));
                        painter.drawPixmap(target, image, source);
                    }
            }
			// locked areas, stairs, doors, etc
            if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].locked != 0 && dataExchanger->editTool == EDITMODE_LOCK) {
                    // transparent rectangle
					//painter.setBrush(QColor(255, 0, 0, 30));
					//painter.drawRect(i*16, j*16, 16, 16);
                    // red border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(255, 0, 0, 255));
                    painter.drawRect(i*16*dataExchanger->zoom, j*16*dataExchanger->zoom, 16*dataExchanger->zoom, 16*dataExchanger->zoom);
					// terrain type icon
					QString terrainIcon;
					QResource::registerResource("resources/icons/icons.qrc");
                    if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].locked == 1) {
						terrainIcon = QString::fromUtf8(":/toolbar_icons/Lock"); // solid
                    } else if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].locked == 2) {
						terrainIcon = QString(":/toolbar_icons/stairs.png"); // stairs
                    } else if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].locked == 3) {
						terrainIcon = QString(":/toolbar_icons/object-order-lower.png"); // door
                    } else if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].locked == 4) {
						terrainIcon = QString(":/toolbar_icons/edit-delete.png"); // spikes
                    } else if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].locked == 5) {
						terrainIcon = QString(":/toolbar_icons/flag-blue.png"); // water
                    } else if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].locked == 6) {
						terrainIcon = QString(":/toolbar_icons/flag-green.png"); // ice
                    } else if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].locked == 7) {
						terrainIcon = QString(":/toolbar_icons/arrow-left.png"); // move left
                    } else if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].locked == 8) {
						terrainIcon = QString(":/toolbar_icons/arrow-right.png"); // move right
                    } else if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].locked == 9) {
						terrainIcon = QString(":/toolbar_icons/arrow-down.png"); // move right
                    } else if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].locked == 10) {
						terrainIcon = QString(":/toolbar_icons/system-switch-user.png"); // move right
                    } else if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].locked == 11) {
						terrainIcon = QString(":/toolbar_icons/Save"); // move right
					}
					QPixmap terrainImage(terrainIcon);
					if (terrainImage.isNull()) {
						printf("ERROR: EditorArea::paintEvent - terrainType - Could not load image file '%s'\n", qPrintable(terrainIcon));
					} else {
						terrainIcon.resize(16);
						painter.setOpacity(0.7);
						QRectF target(QPoint(i*16*dataExchanger->zoom, j*16*dataExchanger->zoom), QSize(16*dataExchanger->zoom, 16*dataExchanger->zoom));
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
        pos = i*16*dataExchanger->zoom-1;
        //QLineF line(0, 800, 16, 800);
        // linhas horizontais
        line = QLineF(pos, 0, pos, MAP_H*16*dataExchanger->zoom-1);
        painter.drawLine(line);
    }
    for (i=1; i<MAP_H; i++) {
        pos = i*16*dataExchanger->zoom-1;
        //QLineF line(0, 800, 16, 800);
        // linhas verticais
        line = QLineF(0, pos, MAP_W*16*dataExchanger->zoom-1, pos);
        painter.drawLine(line);
    }



    if (dataExchanger->show_teleporters_flag == true) {
        // draw links
        //printf("editoMode: %d, EDITMODE_NORMAL: %d, editTool: %d, EDITMODE_LINK_DEST: %d\n", dataExchanger->editMode, EDITMODE_NORMAL, dataExchanger->editTool, EDITMODE_LINK_DEST);
        if (dataExchanger->editMode == EDITMODE_NORMAL && (dataExchanger->editTool == EDITMODE_LINK_DEST || dataExchanger->editTool == EDITMODE_LINK)) {
            for (int i=0; i<STAGE_MAX_LINKS; i++) {
                if (dataExchanger->currentMap ==  stage_data.stages[dataExchanger->currentStage].links[i].id_map_origin) {
                    // transparent blue rectangle
                    if (stage_data.stages[dataExchanger->currentStage].links[i].type == LINK_FINAL_BOSS_ROOM) {
                        painter.setBrush(QColor(255, 0, 0, 180));
                    } else if (stage_data.stages[dataExchanger->currentStage].links[i].type == LINK_TELEPORTER) {
                        painter.setBrush(QColor(0, 255, 0, 180));
                    } else if (stage_data.stages[dataExchanger->currentStage].links[i].type == LINK_BOSS_TELEPORTER) {
                        painter.setBrush(QColor(255, 100, 0, 180));
                    } else {
                        painter.setBrush(QColor(0, 0, 255, 180));
                    }
                    painter.drawRect(stage_data.stages[dataExchanger->currentStage].links[i].pos_origin.x*16*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].links[i].pos_origin.y *16*dataExchanger->zoom, 16*dataExchanger->zoom*stage_data.stages[dataExchanger->currentStage].links[i].size, 16*dataExchanger->zoom);
                    // blue border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(0, 0, 255, 255));
                    painter.drawRect(stage_data.stages[dataExchanger->currentStage].links[i].pos_origin.x*16*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].links[i].pos_origin.y*16*dataExchanger->zoom, 16*dataExchanger->zoom*stage_data.stages[dataExchanger->currentStage].links[i].size, 16*dataExchanger->zoom);
                    painter.setPen(QColor(255, 255, 255, 255));
                    painter.drawText(stage_data.stages[dataExchanger->currentStage].links[i].pos_origin.x*16*dataExchanger->zoom, (stage_data.stages[dataExchanger->currentStage].links[i].pos_origin.y+1)*16*dataExchanger->zoom, QString::number(i));
                } else if (dataExchanger->currentMap == stage_data.stages[dataExchanger->currentStage].links[i].id_map_destiny) {
                    // transparent cyan rectangle
                    if (stage_data.stages[dataExchanger->currentStage].links[i].type == LINK_FINAL_BOSS_ROOM) {
                        painter.setBrush(QColor(160, 60, 60, 180));
                    } else if (stage_data.stages[dataExchanger->currentStage].links[i].type == LINK_TELEPORTER) {
                        painter.setBrush(QColor(60, 160, 60, 180));
                    } else if (stage_data.stages[dataExchanger->currentStage].links[i].type == LINK_BOSS_TELEPORTER) {
                        painter.setBrush(QColor(160, 60, 60, 180));
                    } else {
                        painter.setBrush(QColor(0, 255, 255, 180));
                    }
                    painter.drawRect(stage_data.stages[dataExchanger->currentStage].links[i].pos_destiny.x*16*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].links[i].pos_destiny.y*16*dataExchanger->zoom, 16*dataExchanger->zoom*stage_data.stages[dataExchanger->currentStage].links[i].size, 16*dataExchanger->zoom);
                    // cyan border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(0, 255, 255, 255));
                    painter.drawRect(stage_data.stages[dataExchanger->currentStage].links[i].pos_destiny.x*16*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].links[i].pos_destiny.y*16*dataExchanger->zoom, 16*dataExchanger->zoom*stage_data.stages[dataExchanger->currentStage].links[i].size, 16*dataExchanger->zoom);
                    painter.setPen(QColor(0, 0, 0, 255));
                    painter.drawText(stage_data.stages[dataExchanger->currentStage].links[i].pos_destiny.x*16*dataExchanger->zoom, (stage_data.stages[dataExchanger->currentStage].links[i].pos_destiny.y+1)*16*dataExchanger->zoom, QString::number(i));
                }
            }
        }
    }



    if (dataExchanger->show_npcs_flag == true) {
        /// draw NPCs
        for (int i=0; i<FS_MAX_MAP_NPCS; i++) {

            //std::cout << "EditorArea::paintEvent #5.0.A [" << i << "]" << std::endl;
            int npc_id = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc;
            if (npc_id >= FS_GAME_MAX_NPCS || npc_id < 0) {
                //std::cout << "EditorArea::paintEvent #5.0.B - stage[" << dataExchanger->currentStage << "], map[" << dataExchanger->currentMap << "], i[" << i << "], id_npc[" << npc_id << "] BAD" << std::endl;
                stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc = -1;
                continue;
    //        } else {
    //            std::cout << "EditorArea::paintEvent #5.0.B - i[" << i << "], id_npc[" << npc_id << "] GOOD" << std::endl;
            }

            //std::cout << "EditorArea::paintEvent #5.0.C graphic_filename[" << game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].graphic_filename << "]" << std::endl;
            std::string filename = FILEPATH + "/data/images/sprites/enemies/" + game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].graphic_filename;
            if (filename.length() <= 0) {
                continue;
            }


            QPixmap temp_image(filename.c_str());
            if (!temp_image.isNull()) {

                int total_w = game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].frame_size.width*dataExchanger->zoom;
                int total_h = game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].frame_size.height*dataExchanger->zoom;
                int sprite_adjust_x = game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].sprites_pos_bg.x;
                int sprite_adjust_y = game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].sprites_pos_bg.y;


                std::string npc_bg_file(game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].bg_graphic_filename);
                if (npc_bg_file.length() > 0) {
                    std::string _bg_graphic_filename = FILEPATH + "/data/images/sprites/backgrounds/" + npc_bg_file;
                    QPixmap bg_image(_bg_graphic_filename.c_str());

                    // calculate total image size of background exists
                    if (!bg_image.isNull()) {
                        total_w = bg_image.width();
                        total_h = bg_image.height();
                    }
                    if (!bg_image.isNull()) {
                        QRectF bg_target(QPoint(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.x*16*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.y*16*dataExchanger->zoom), QSize(total_w, total_h));
                        QRectF bg_source(QRectF(QPoint(0, 0), QSize(bg_image.width(), bg_image.height())));
                        painter.drawPixmap(bg_target, bg_image, bg_source);
                    }
                }

                QRectF target(QPoint((stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.x*16+sprite_adjust_x)*dataExchanger->zoom, (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.y*16+sprite_adjust_y)*dataExchanger->zoom), QSize(total_w, total_h));
                QRectF source;
                if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].direction != ANIM_DIRECTION_RIGHT || temp_image.height() <= game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].frame_size.height) {
                    source = QRectF(QPoint(0, 0), QSize(game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].frame_size.width, game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].frame_size.height));
                } else {
                    source = QRectF(QPoint(0, game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].frame_size.height), QSize(game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].frame_size.width, game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].frame_size.height));
                }
                if (stage_data.stages[dataExchanger->currentStage].boss.id_npc == stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc) {
                    // transparent green rectangle
                    painter.setBrush(QColor(0, 255, 0, 180));
                    painter.drawRect(target);
                } else if (game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].is_boss) {
                    // transparent orange rectangle
                    painter.setBrush(QColor(255, 128, 35, 200));
                    painter.drawRect(target);
                } else if (game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].is_sub_boss) {
                    // transparent bright-orange rectangle
                    painter.setBrush(QColor(241, 188, 87, 200));
                    painter.drawRect(target);
                }
                painter.drawPixmap(target, temp_image, source);
            }
        }
    }

    if (dataExchanger->show_objects_flag == true) {
        /// draw objects
        //std::cout << "################### START LOOP" << std::endl;
        for (int i=0; i<FS_MAX_MAP_OBJECTS; i++) {
            int obj_id = (int)stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].id_object;
            if (obj_id != -1) {
                //if (obj_id == 20) { std::cout << "************************** paintEvent - draw_objects[" << i << "].id: " << obj_id << std::endl; }
                std::string filename = FILEPATH + "/data/images/sprites/objects/" + game_data.objects[obj_id].graphic_filename;
                QPixmap temp_image(filename.c_str());
                if (temp_image.isNull()) {
                    std::cout << "****************** Could not load file '" << filename.c_str() << "'" << std::endl;
                    painter.setBrush(QColor(255, 255, 255, 180));
                    painter.drawRect(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.x*16*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.y*16*dataExchanger->zoom, 16*dataExchanger->zoom, 16*dataExchanger->zoom);
                } else {
                    int obj_type = game_data.objects[obj_id].type;
                    int obj_direction = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].direction;
                    if (obj_type == OBJ_RAY_HORIZONTAL) {
                        int graphic_pos_x = 0;
                        int graphic_pos_y = game_data.objects[obj_id].size.height;
                        if (obj_direction == ANIM_DIRECTION_LEFT) {
                            graphic_pos_x = game_data.objects[obj_id].size.width - TILESIZE;
                            graphic_pos_y = 0;
                        }
                        //std::cout << "OBJ_RAY_HORIZONTAL - obj_direction: " << obj_direction << ", graphic_pos_x: " << graphic_pos_x << ", graphic_pos_y: " << graphic_pos_y << std::endl;
                        QRectF target(QPoint(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.x*TILESIZE*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.y*TILESIZE*dataExchanger->zoom), QSize(TILESIZE*dataExchanger->zoom, game_data.objects[obj_id].size.height*dataExchanger->zoom));
                        QRectF source(QPoint(graphic_pos_x, graphic_pos_y), QSize(TILESIZE, game_data.objects[obj_id].size.height));
                        painter.drawPixmap(target, temp_image, source);
                    } else if (obj_type == OBJ_RAY_VERTICAL) {
                        int graphic_pos_y = game_data.objects[obj_id].size.height - TILESIZE;
                        //std::cout << "OBJ_RAY_HORIZONTAL - graphic_pos_y: " << graphic_pos_y << std::endl;
                        QRectF target(QPoint(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.x*16*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.y*16*dataExchanger->zoom), QSize(game_data.objects[obj_id].size.width*dataExchanger->zoom, game_data.objects[obj_id].size.height*dataExchanger->zoom));
                        QRectF source(QPoint(0, graphic_pos_y), QSize(game_data.objects[obj_id].size.width, game_data.objects[obj_id].size.height));
                        painter.drawPixmap(target, temp_image, source);
                    } else if (obj_type == OBJ_DEATHRAY_HORIZONTAL && obj_direction == ANIM_DIRECTION_LEFT) {
                        QRectF target(QPoint(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.x*TILESIZE*dataExchanger->zoom - (game_data.objects[obj_id].size.width-TILESIZE)*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.y*16*dataExchanger->zoom), QSize(game_data.objects[obj_id].size.width*dataExchanger->zoom, game_data.objects[obj_id].size.height*dataExchanger->zoom));
                        QRectF source(QPoint(0, 0), QSize(game_data.objects[obj_id].size.width, game_data.objects[obj_id].size.height));
                        painter.drawPixmap(target, temp_image, source);
                    } else {
                        QRectF target(QPoint(stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.x*16*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.y*16*dataExchanger->zoom), QSize(game_data.objects[obj_id].size.width*dataExchanger->zoom, game_data.objects[obj_id].size.height*dataExchanger->zoom));
                        QRectF source;
                        if (obj_direction == ANIM_DIRECTION_RIGHT && temp_image.height() >= game_data.objects[obj_id].size.height) {
                            source = QRectF(QPoint(0, game_data.objects[obj_id].size.height), QSize(game_data.objects[obj_id].size.width, game_data.objects[obj_id].size.height));
                        } else {
                            source = QRectF(QPoint(0, 0), QSize(game_data.objects[obj_id].size.width, game_data.objects[obj_id].size.height));
                        }
                        painter.drawPixmap(target, temp_image, source);
                    }
                }
                // draw object-teleporter origin
                if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].map_dest != -1) {
                    int obj_w = game_data.objects[obj_id].size.width;
                    int obj_h = game_data.objects[obj_id].size.height;
                    std::cout << "OBJ.w: " << obj_w << ", obj_h: " << obj_h << std::endl;
                    int dest_x = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.x*TILESIZE*dataExchanger->zoom + (obj_w/2)*dataExchanger->zoom - TILESIZE/2;
                    int dest_y = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.y*TILESIZE*dataExchanger->zoom + (obj_h/2)*dataExchanger->zoom - TILESIZE/2;
                    std::cout << "DRAW OBJECT TELEPORTER ORIGIN[" << i << "] - map: " << (int)stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].map_dest  << ", x: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.x << ", y: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.y << std::endl;
                    if (game_data.objects[obj_id].type == OBJ_FINAL_BOSS_TELEPORTER) {
                        painter.setBrush(QColor(160, 60, 60, 180));
                    } else {
                        painter.setBrush(QColor(60, 160, 60, 180));
                    }
                    painter.drawEllipse(dest_x, dest_y, 16*dataExchanger->zoom, 16*dataExchanger->zoom);
                    // cyan border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(0, 255, 255, 255));
                    painter.drawEllipse(dest_x, dest_y, 16*dataExchanger->zoom, 16*dataExchanger->zoom);
                    painter.setPen(QColor(0, 0, 0, 255));
                    painter.drawText(dest_x+3, dest_y+TILESIZE-3, QString::number(i));
                }

            }
        }

        for (int k=0; k<FS_STAGE_MAX_MAPS; k++) {
            for (int i=0; i<FS_MAX_MAP_OBJECTS; i++) {
                // draw teleport destiny links
                //std::cout << "OBJ[" << i << "].map_dest: " << (int)stage_data.stages[dataExchanger->currentStage].maps[k].map_objects[i].map_dest << ", currentMap: " << k << std::endl;
                if (stage_data.stages[dataExchanger->currentStage].maps[k].map_objects[i].map_dest == dataExchanger->currentMap) {
                    int obj_id = stage_data.stages[dataExchanger->currentStage].maps[k].map_objects[i].id_object;
                    std::cout << "## EDITORAREA::paintEvent - teleporter_obj - x: " << (int)stage_data.stages[dataExchanger->currentStage].maps[k].map_objects[i].link_dest.x << ", y: " << (int)stage_data.stages[dataExchanger->currentStage].maps[k].map_objects[i].link_dest.y << std::endl;
                    if (game_data.objects[obj_id].type == OBJ_FINAL_BOSS_TELEPORTER) {
                        painter.setBrush(QColor(160, 60, 60, 180));
                    } else {
                        painter.setBrush(QColor(60, 160, 60, 180));
                    }
                    painter.drawEllipse(stage_data.stages[dataExchanger->currentStage].maps[k].map_objects[i].link_dest.x*16*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].maps[k].map_objects[i].link_dest.y*16*dataExchanger->zoom, 16*dataExchanger->zoom, 16*dataExchanger->zoom);
                    // cyan border
                    painter.setBrush(Qt::NoBrush);
                    painter.setPen(QColor(0, 255, 255, 255));
                    painter.drawEllipse(stage_data.stages[dataExchanger->currentStage].maps[k].map_objects[i].link_dest.x*16*dataExchanger->zoom, stage_data.stages[dataExchanger->currentStage].maps[k].map_objects[i].link_dest.y*16*dataExchanger->zoom, 16*dataExchanger->zoom, 16*dataExchanger->zoom);
                    painter.setPen(QColor(0, 0, 0, 255));
                    painter.drawText(stage_data.stages[dataExchanger->currentStage].maps[k].map_objects[i].link_dest.x*16*dataExchanger->zoom + 3*dataExchanger->zoom, (stage_data.stages[dataExchanger->currentStage].maps[k].map_objects[i].link_dest.y+1)*16*dataExchanger->zoom -2*dataExchanger->zoom, QString::number(i));
                }
            }
        }
    }


    QSize resizeMe(MAP_W*16*dataExchanger->zoom, MAP_H*16*dataExchanger->zoom);
    this->resize(resizeMe);
    myParent->adjustSize();

}

void EditorArea::mouseMoveEvent(QMouseEvent *event) {
	QPoint pnt = event->pos();
        if (editor_selectedTileX != pnt.x()/(16*dataExchanger->zoom) || editor_selectedTileY != pnt.y()/(16*dataExchanger->zoom)) {
		mousePressEvent(event);
	}
}

void EditorArea::mousePressEvent(QMouseEvent *event) {
    if (mouse_released == false && (dataExchanger->editTool == EDITMODE_LINK || dataExchanger->editTool == EDITMODE_LINK_DEST || dataExchanger->editMode == EDITMODE_NPC || dataExchanger->editMode == EDITMODE_OBJECT || dataExchanger->editMode == EDITMODE_SET_BOSS || dataExchanger->editMode == EDITMODE_SET_SUBBOSS)) {
		return;
	}

    QPoint pnt = event->pos();
    editor_selectedTileX = pnt.x()/(16*dataExchanger->zoom);
    editor_selectedTileY = pnt.y()/(16*dataExchanger->zoom);

	if (dataExchanger->editMode == EDITMODE_NORMAL) {
		if (dataExchanger->editTool == EDITMODE_NORMAL) {
			if (dataExchanger->layerLevel == 1) {
                stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.x = dataExchanger->getPalleteX();
                stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.y = dataExchanger->getPalleteY();
			} else if (dataExchanger->layerLevel == 3) {
				printf(">> EditorArea::mousePressEvent - placing tile 3\n");
                stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.x = dataExchanger->getPalleteX();
                stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.y = dataExchanger->getPalleteY();
			}
		} else if (dataExchanger->editTool == EDITMODE_ERASER) {
			if (dataExchanger->layerLevel == 1) {
                stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.x = -1;
                stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.y = -1;
			} else if (dataExchanger->layerLevel == 3) {
                stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.x = -1;
                stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.y = -1;
			}
		} else if (dataExchanger->editTool == EDITMODE_LOCK) {
            if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].locked == 0) {
                stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].locked = dataExchanger->terrainType;
			} else {
                stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].locked = 0;
			}

		} else if (dataExchanger->editTool == EDITMODE_FILL) {
			fill_area();






		// first click on origin link
		} else if (dataExchanger->editTool == EDITMODE_LINK && tempX == -1) {
			// checks if a link in this position already exits to remove it
			bool removed_link = false;
			for (int i=0; i<STAGE_MAX_LINKS; i++) {
                if (stage_data.stages[dataExchanger->currentStage].links[i].id_map_origin == dataExchanger->currentMap) {
                    if (stage_data.stages[dataExchanger->currentStage].links[i].pos_origin.x == editor_selectedTileX && stage_data.stages[dataExchanger->currentStage].links[i].pos_origin.y == editor_selectedTileY) {
                        stage_data.stages[dataExchanger->currentStage].links[i].id_map_destiny = -1;
                        stage_data.stages[dataExchanger->currentStage].links[i].id_map_origin = -1;
						std::cout << "######## -> editorArea::mousePress - removed link 1" << std::endl;
						removed_link = true;
						break;
					}
                } else if (stage_data.stages[dataExchanger->currentStage].links[i].id_map_destiny == dataExchanger->currentMap) {
                    if (stage_data.stages[dataExchanger->currentStage].links[i].pos_destiny.x == editor_selectedTileX && stage_data.stages[dataExchanger->currentStage].links[i].pos_destiny.y == editor_selectedTileY) {
                        stage_data.stages[dataExchanger->currentStage].links[i].id_map_destiny = -1;
                        stage_data.stages[dataExchanger->currentStage].links[i].id_map_origin = -1;
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
                    std::cout << ">> DEBUG - links[" << i << "].id_map_destiny: " << stage_data.stages[dataExchanger->currentStage].links[i].id_map_destiny << ", id_map_origin: " << stage_data.stages[dataExchanger->currentStage].links[i].id_map_origin << std::endl;
                    if (stage_data.stages[dataExchanger->currentStage].links[i].id_map_destiny == -1 && stage_data.stages[dataExchanger->currentStage].links[i].id_map_origin == -1) {
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
                if (dataExchanger->editTool == EDITMODE_LINK && tempX == -1 && dataExchanger->link_type == LINK_FINAL_BOSS_ROOM) {
                    std::cout << "########### -> editorArea::mousePress - adding LAST BOSS point" << std::endl;
                    stage_data.stages[dataExchanger->currentStage].links[link_n].id_map_origin = link_map_origin;
                    stage_data.stages[dataExchanger->currentStage].links[link_n].pos_origin.x = editor_selectedTileX;
                    stage_data.stages[dataExchanger->currentStage].links[link_n].pos_origin.y = editor_selectedTileY;
                    stage_data.stages[dataExchanger->currentStage].links[link_n].size = link_size;
                    stage_data.stages[dataExchanger->currentStage].links[link_n].id_map_destiny = dataExchanger->currentMap;
                    stage_data.stages[dataExchanger->currentStage].links[link_n].pos_destiny.x = editor_selectedTileX;
                    stage_data.stages[dataExchanger->currentStage].links[link_n].pos_destiny.y = editor_selectedTileY;
                    /// @TODO: this must come from editor properties
                    stage_data.stages[dataExchanger->currentStage].links[link_n].bidirecional = false;
                    stage_data.stages[dataExchanger->currentStage].links[link_n].is_door = false;
                    stage_data.stages[dataExchanger->currentStage].links[link_n].type = dataExchanger->link_type;
                } else {
                    std::cout << "########### -> editorArea::mousePress - adding link ORIGIN at slot[" << link_n << "] - PART 1" << std::endl;
                    tempX = editor_selectedTileX;
                    tempY = editor_selectedTileY;
                }
			}
		} else if (dataExchanger->editTool == EDITMODE_LINK_DEST) {
			std::cout << "########### -> editorArea::mousePress - adding link - PART 3" << std::endl;
			int link_n = -1;
			for (int i=0; i<STAGE_MAX_LINKS; i++) {
                if (stage_data.stages[dataExchanger->currentStage].links[i].id_map_destiny == -1 && stage_data.stages[dataExchanger->currentStage].links[i].id_map_origin == -1) {
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
            stage_data.stages[dataExchanger->currentStage].links[link_n].id_map_origin = link_map_origin;
            stage_data.stages[dataExchanger->currentStage].links[link_n].pos_origin.x = link_pos_x;
            stage_data.stages[dataExchanger->currentStage].links[link_n].pos_origin.y = link_pos_y;
            stage_data.stages[dataExchanger->currentStage].links[link_n].size = link_size;
            stage_data.stages[dataExchanger->currentStage].links[link_n].id_map_destiny = dataExchanger->currentMap;
            stage_data.stages[dataExchanger->currentStage].links[link_n].pos_destiny.x = editor_selectedTileX;
            stage_data.stages[dataExchanger->currentStage].links[link_n].pos_destiny.y = editor_selectedTileY;
			/// @TODO: this must come from editor properties
            stage_data.stages[dataExchanger->currentStage].links[link_n].bidirecional = dataExchanger->link_bidi;
            stage_data.stages[dataExchanger->currentStage].links[link_n].is_door = dataExchanger->link_is_door;
            stage_data.stages[dataExchanger->currentStage].links[link_n].type = dataExchanger->link_type;

			dataExchanger->editTool = EDITMODE_LINK;
			repaint();
            QApplication::setOverrideCursor(Qt::ArrowCursor);
			return;
		}



    } else if (dataExchanger->editMode == EDITMODE_SET_BOSS) {
        std::cout << ">> EditorArea::mousePressEvent - EDITMODE_SET_BOSS" << std::endl;
        // search if there is an existing NPC in ths position, and if yes, set as boss
        for (int i=0; i<MAX_MAP_NPC_N; i++) {
            if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc != -1 && stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.x == editor_selectedTileX && stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.y == editor_selectedTileY) {
                stage_data.stages[dataExchanger->currentStage].boss.id_npc = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc;
                sprintf(stage_data.stages[dataExchanger->currentStage].boss.name, "%s", game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].name);
                break;
            }
        }


    } else if (dataExchanger->editMode == EDITMODE_SET_SUBBOSS) {
        std::cout << ">> EditorArea::mousePressEvent - EDITMODE_SET_BOSS" << std::endl;
        // search if there is an existing NPC in ths position, and if yes, set as sub-boss
        for (int i=0; i<MAX_MAP_NPC_N; i++) {
            if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc != -1 && stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.x == editor_selectedTileX && stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.y == editor_selectedTileY) {
                std::cout << ">> EditorArea::mousePressEvent - EDITMODE_SET_BOSS - FOUND NPC" << std::endl;
                game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].is_sub_boss = !game_data.game_npcs[stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc].is_sub_boss;
                break;
            }
        }



/// @TODO: convert to new file-system ///

	} else if (dataExchanger->editMode == EDITMODE_NPC) {
		printf(">> EditorArea::mousePressEvent - EDITMODE_NPC\n");

		int found_npc = -1;
		// search if there is an existing NPC in ths position, and if yes, remove it
		for (int i=0; i<MAX_MAP_NPC_N; i++) {
            if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc != -1 && stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.x == editor_selectedTileX && stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.y == editor_selectedTileY) {
				found_npc = i;
				break;
			}
		}
		// clean old-format trash
        for (int i=0; i<FS_MAX_MAP_OBJECTS; i++) {
            if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.x == -1 && stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.y == -1) {
                std::cout << "[OBJECT] reseting free-slot - i: " << i << ", id: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc << ", x: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.x << ", y: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.y << std::endl;
                stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc = -1;
			}
		}


		if (dataExchanger->editTool == EDITMODE_ERASER && found_npc != -1) {
			std::cout << "remove npc - slot: " << found_npc << std::endl;
            stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[found_npc].id_npc = -1;
		} else if (dataExchanger->editTool == EDITMODE_NORMAL && found_npc == -1 && dataExchanger->selectedNPC != -1) {
			// search for free slot
			for (int i=0; i<MAX_MAP_NPC_N; i++) {
                //std::cout << "searching for free-slot - i: " << i << ", id: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc << ", x: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.x << ", y: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.y << std::endl;
                if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc == -1) {

                    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc = dataExchanger->selectedNPC;

                    std::cout << ">> added npc to map - stage[" << dataExchanger->currentStage << "], map[" << dataExchanger->currentMap << "], slot[" << i << "], id[" << dataExchanger->selectedNPC << "], set_value[" << (int)stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].id_npc << "]" << std::endl;

                    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.x = editor_selectedTileX;
                    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].start_point.y = editor_selectedTileY;
                    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_npcs[i].direction = dataExchanger->npc_direction;
					repaint();
					return;
				}
			}
			QMessageBox msgBox;
			msgBox.setText("No free slot to place NPC in map.");
			msgBox.exec();
			return;
		} else if (dataExchanger->editTool == EDITMODE_NORMAL && found_npc != -1 && dataExchanger->selectedNPC != -1) {
			printf(">> EditorArea::mousePressEvent - Adding NPC - place already taken\n");
		}



	} else if (dataExchanger->editMode == EDITMODE_OBJECT) {


		printf(">> EditorArea::mousePressEvent - EDITMODE_OBJECT");

		int found_object = -1;
		// search if there is an existing object in ths position, and if yes, remove it
        for (int i=0; i<FS_MAX_MAP_OBJECTS; i++) {
            if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].id_object != -1 && stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.x == editor_selectedTileX && stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.y == editor_selectedTileY) {
				found_object = i;
				break;
			}
		}
		// clean old-format trash
        for (int i=0; i<FS_MAX_MAP_OBJECTS; i++) {
            if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.x == -1 && stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.y == -1) {
                std::cout << "reseting free-slot - i: " << i << ", id: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].id_object << ", x: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.x << ", y: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.y << std::endl;
                stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].id_object = -1;
			}
		}





		if (dataExchanger->editTool == EDITMODE_ERASER && found_object != -1) {
			std::cout << "remove object - slot: " << found_object << std::endl;
            stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[found_object].id_object = -1;
		} else if (dataExchanger->editTool == EDITMODE_NORMAL && found_object == -1 && dataExchanger->selectedNPC != -1) {
			// search for free slot
            for (int i=0; i<FS_MAX_MAP_OBJECTS; i++) {
                std::cout << "[OBJECT] searching for free-slot - i: " << i << ", id: " << (int)stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].id_object << ", x: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.x << ", y: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.y << std::endl;
                if (stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].id_object == -1) {
                    std::cout << "add object - slot: " << i << ", id: " << (int)dataExchanger->selectedNPC << std::endl;
                    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].id_object = dataExchanger->selectedNPC;
                    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.x = editor_selectedTileX;
                    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].start_point.y = editor_selectedTileY;
                    stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].direction = dataExchanger->object_direction;
                    editor_selected_object_pos_map = dataExchanger->currentMap;
                    // se item é teleportador, deve entrar no modo de colocar link de object
                    if (game_data.objects[dataExchanger->selectedNPC].type == OBJ_BOSS_TELEPORTER || game_data.objects[dataExchanger->selectedNPC].type == OBJ_FINAL_BOSS_TELEPORTER) {
                        editor_selected_object_pos = i;
                        std::cout << "SET editor_selected_object_pos: " << editor_selected_object_pos << std::endl;
                        dataExchanger->editTool = EDITMODE_OBJECT_LINK_PLACING;
                        QApplication::setOverrideCursor(Qt::CrossCursor);
                        // @TODO - desabilita todos os modos, não pode sair no meio de object-link-placing
                        // @TODO - desabilitar mudar de estágio também (e isso tem que ser feito no link tb)
                    } else {
                        stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].map_objects[i].map_dest = -1;
                    }
					repaint();
					return;
				}
			}
			QMessageBox msgBox;
			msgBox.setText("No free slot to place object in map.");
			msgBox.exec();
			return;
		} else if (dataExchanger->editTool == EDITMODE_NORMAL && found_object != -1 && dataExchanger->selectedNPC != -1) {
			printf(">> EditorArea::mousePressEvent - Adding object - place already taken\n");
        } else if (dataExchanger->editTool == EDITMODE_OBJECT_LINK_PLACING) {
            std::cout << "USE editor_selected_object_pos_map: " << editor_selected_object_pos_map << ", editor_selected_object_pos: " << editor_selected_object_pos << ", editor_selectedTileX: " << editor_selectedTileX << ", editor_selectedTileY: " << editor_selectedTileY <<    std::endl;
            stage_data.stages[dataExchanger->currentStage].maps[editor_selected_object_pos_map].map_objects[editor_selected_object_pos].link_dest.x = editor_selectedTileX;
            stage_data.stages[dataExchanger->currentStage].maps[editor_selected_object_pos_map].map_objects[editor_selected_object_pos].link_dest.y = editor_selectedTileY;
            stage_data.stages[dataExchanger->currentStage].maps[editor_selected_object_pos_map].map_objects[editor_selected_object_pos].map_dest = dataExchanger->currentMap;
            std::cout << "[TELEPORTER] SET map: " << (int)stage_data.stages[dataExchanger->currentStage].maps[editor_selected_object_pos_map].map_objects[editor_selected_object_pos].map_dest << ", x: " << (int)stage_data.stages[dataExchanger->currentStage].maps[editor_selected_object_pos_map].map_objects[editor_selected_object_pos].link_dest.x << ", y: " << (int)stage_data.stages[dataExchanger->currentStage].maps[editor_selected_object_pos_map].map_objects[editor_selected_object_pos].link_dest.y << std::endl;
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        }





        std::cout << ">> EditorArea::mousePressEvent - EDITMODE_OBJECT" << std::endl;

	}
    temp = 1;
    repaint();
}


void EditorArea::mouseReleaseEvent(QMouseEvent *event) {
	QPoint pnt = event->pos();
	editor_selectedTileX = pnt.x()/(16*dataExchanger->zoom);
	editor_selectedTileY = pnt.y()/(16*dataExchanger->zoom);

    if (dataExchanger->editTool == EDITMODE_LINK && tempX != -1) {// && dataExchanger->link_type != LINK_FINAL_BOSS_ROOM) {
		std::cout << "########### -> editorArea::mousePress - adding link - PART 2" << std::endl;
		// TODO: add link
		dataExchanger->editTool = EDITMODE_LINK_DEST;
        QApplication::setOverrideCursor(Qt::CrossCursor);

		link_map_origin = dataExchanger->currentMap;
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
    } else {
        std::cout << "########### -> editorArea::mousePress - ignore mouse release" << std::endl;
        std::cout << "######### edit_move: " << dataExchanger->editTool << ", tempX: " << tempX << std::endl;
    }
	mouse_released = true;
}

void EditorArea::fill_area() {
	int i, j;
	int filled1=0;
	int filled2=0;
	int originalX, originalY;
    short int searchX, searchY;

	if (dataExchanger->layerLevel == 1) {
        originalX = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.x;
        originalY = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile1.y;
	} else if (dataExchanger->layerLevel == 3) {
        originalX = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.x;
        originalY = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[editor_selectedTileX][editor_selectedTileY].tile3.y;
	}
	// área a esquerda
	for (i=editor_selectedTileX; i>=0; i--) {
		// área acima
		filled1 = 0;
		filled2 = 0;
		for (j=editor_selectedTileY; j>=0; j--) {
			if (dataExchanger->layerLevel == 1) {
                searchX = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.x;
                searchY = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.y;
			} else if (dataExchanger->layerLevel == 3) {
                searchX = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.x;
                searchY = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.y;
			}
			//printf("DEBUG.EditorArea::i: %d, j: %d, fill_area - tile.x: %d, tile;y: %d, originalX: %d, originalY: %d\n", i, j, map.tiles[i][j].tile1.x, map.tiles[i][j].tile1.y, originalX, originalY);
            if (searchX == originalX && searchY == originalY) {
                searchX = dataExchanger->getPalleteX();
                searchY = dataExchanger->getPalleteY();
				filled1 = 1;
			} else {
				printf("1.not filling\n");
				break;
			}
		}
		// área abaixo
		for (j=editor_selectedTileY+1; j<MAP_H; j++) {
			if (dataExchanger->layerLevel == 1) {
                searchX = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.x;
                searchY = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.y;
			} else if (dataExchanger->layerLevel == 3) {
                searchX = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.x;
                searchY = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.y;
			}
            if (searchX == originalX && searchY == originalY) {
                searchX = dataExchanger->getPalleteX();
                searchY = dataExchanger->getPalleteY();
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
			if (dataExchanger->layerLevel == 1) {
                searchX = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.x;
                searchY = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.y;
			} else if (dataExchanger->layerLevel == 3) {
                searchX = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.x;
                searchY = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.y;
			}
            if (searchX == originalX && searchY == originalY) {
                searchX = dataExchanger->getPalleteX();
                searchY = dataExchanger->getPalleteY();
				filled1 = 1;
			} else {
				printf("3.not filling\n");
				break;
			}
		}
		// área abaixo
		for (j=editor_selectedTileY+1; j<MAP_H; j++) {
			if (dataExchanger->layerLevel == 1) {
                searchX = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.x;
                searchY = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile1.y;
			} else if (dataExchanger->layerLevel == 3) {
                searchX = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.x;
                searchY = stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].tiles[i][j].tile3.y;
			}
            if (searchX == originalX && searchY == originalY) {
                searchX = dataExchanger->getPalleteX();
                searchY = dataExchanger->getPalleteY();
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



void EditorArea::saveGame(int game_n) {
    //std::cout << "EditorArea::saveGame ******** backgrounds[0].filename: " << stage_data.stages[dataExchanger->currentStage].maps[dataExchanger->currentMap].backgrounds[0].filename << std::endl;

	fio.write_game(game_data);

    // para remover teleporters mal-setados
    /*
    for (int i=0; i<FS_MAX_STAGES; i++) {
        for (int j=0; j<FS_STAGE_MAX_MAPS; j++) {
            for (int k=0; k<FS_MAX_MAP_OBJECTS; k++) {
                stage_data.stages[i].maps[j].map_objects[k].link_dest.x = -1;
                stage_data.stages[i].maps[j].map_objects[k].link_dest.y = -1;
                stage_data.stages[i].maps[j].map_objects[k].map_dest = -1;
            }
        }
    }
    */

    fio.write_all_stages(stage_data);
}


void EditorArea::removeNpcFromMap(int posx, int posy) {
	struct format_v1_0::list_map_npc *temp_list, *item_ant=NULL, *item_aux=NULL;

	// empty npc list
	temp_list = dataExchanger->npc_map_list;
	while (temp_list) {
		if (temp_list->start_point.x == posx && temp_list->start_point.y == posy) {
			item_aux = temp_list->next;
			free(temp_list);
			temp_list = NULL;
			temp_list = item_aux;
			if (item_ant != NULL) {
				item_ant->next = temp_list;
			} else {
				dataExchanger->npc_map_list = temp_list;
			}
		} else {
			item_ant = temp_list;
			temp_list = temp_list->next;
		}
	}
}

void EditorArea::addObjectToMap(int posx, int posy) {
	// checks if there is a npc in this position
	/// @TODO: rework
	/*
	temp_obj_list = obj_list;
	while (temp_obj_list) {
		if (temp_obj_list->type != -1 && temp_obj_list->start_point.x == posx && temp_obj_list->start_point.y == posy) {
			QMessageBox msgBox;
			msgBox.setText("addObjectToMap: There is a Object in this point already");
			msgBox.exec();
			return;
		}
		temp_obj_list = temp_obj_list->next;
	}
	struct format_v1_0::st_object *new_obj = (struct format_v1_0::st_object *)malloc(sizeof(struct format_v1_0::st_object));
	new_obj->direction = 0;
	new_obj->distance = 0;
	sprintf(new_obj->filename, "%s", "");
	new_obj->frame = 0;
	new_obj->framesize_h = 0;
	new_obj->framesize_w = 0;
	new_obj->limit = 0;
	sprintf(new_obj->name, "%s", "");
	new_obj->next = NULL;
	new_obj->position.x = 0;
	new_obj->position.y = 0;
	new_obj->speed = 0;
	new_obj->start_point.x = posx;
	new_obj->start_point.y = posy;
	//printf("EditorArea::addObjectToMap - x: %d, y: %d\n", new_obj->start_point.x, new_obj->start_point.y);
	new_obj->timer = 0;
	new_obj->type = 0;
	if (obj_list == NULL) {
		//printf (">>addObjectToMap - adding to the BEGIN of obj_list\n");
		obj_list = new_obj;
		if (obj_list == NULL) {
			printf (">>addObjectToMap - adding to the BEGIN of obj_list - FAIL!\n");
		} else {
			//printf (">>addObjectToMap - adding to the BEGIN of obj_list - OK!\n");
		}
	} else {
		temp_obj_list = obj_list;
		while (temp_obj_list->next != NULL) {
			temp_obj_list = temp_obj_list->next;
		}
		//printf (">>addObjectToMap - adding to the END of obj_list\n");
		temp_obj_list->next = new_obj;
	}
	dataExchanger->placeObject(posx, posy, new_obj);
	// go to the end of the list to add
	*/

}


