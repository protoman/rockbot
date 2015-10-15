#include <QApplication>
#include <dirent.h>
#include <assert.h>

#include "mediator.h"

int palleteX=0;
int palleteY=0;



#include "../file/format.h"
#include "editortilepallete.h"
#include "editorarea.h"
#include "newgamedialog.h"
#include "mainwindow.h"
#include "loadgamepicker.h"

#include "defines.h"
#include "enum_names.h"
#include "file/file_io.h"

std::string GAMEPATH; // path without GAMES
std::string FILEPATH; // path including GAMES/[GAME]
std::string SAVEPATH;
std::string GAMENAME;

void remove_duplicated()
{
	for (int i=0; i<MAX_STAGES; i++) {
		for (int j=0; j<STAGE_MAX_MAPS; j++) {
			// remove duplicated NPCS
			for (int k=0; k<MAX_MAP_NPC_N; k++) {
				for (int l=0; l<MAX_MAP_NPC_N; l++) {
                    //stage_data.stages[i].maps[j].map_npcs[k].id_npc != -1 && stage_data.stages[i].maps[j].map_npcs[l].id_npc != -1 &&
                    if (Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[l].id_npc != -1 && k != l && Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[k].start_point == Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[l].start_point) {
                        std::cout << ">>> removing duplicated NPC[" << l << "].id[" << Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[l].id_npc << "] in stage[" << i << "].map[" << j << "] <<<" << std::endl;
                        Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[l].id_npc = -1;
                        Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[l].start_point.x = -1;
                        Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[l].start_point.y = -1;
					}
				}
			}
			// remove duplicated OBJECTS
			for (int k=0; k<MAX_MAP_NPC_N; k++) {
				for (int l=0; l<MAX_MAP_NPC_N; l++) {
                    if (Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[k].id_object != -1 && Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[l].id_object != -1 && k != l && Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[k].start_point == Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[l].start_point) {
                        std::cout << "# id1: " << (int)Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[k].id_object << ", id2: " << (int)Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[l].id_object;
                        std::cout << ", pos#1[" << k << "], pos2[" << l << "]";
                        std::cout << ", x1: " << Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[k].start_point.x << ", x2: " << Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[l].start_point.x << ", y1: " << Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[k].start_point.y << ", y2: " << Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[l].start_point.y << std::endl;
                        Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[l].id_object = -1;
                        Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[l].start_point.x = -1;
                        Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[l].start_point.y = -1;
					}
				}
			}
		}
	}
}

void adjust_sprites_size() {
	for (int k=0; k<GAME_MAX_OBJS; k++) {
		for (int l=0; l<ANIM_TYPE_COUNT; l++) {
			for (int m=0; m<ANIM_FRAMES_COUNT; m++) {
                Mediator::get_instance()->game_data.game_npcs[k].sprites[l][m].colision_rect.x = 0;
                Mediator::get_instance()->game_data.game_npcs[k].sprites[l][m].colision_rect.y = 0;
                Mediator::get_instance()->game_data.game_npcs[k].sprites[l][m].colision_rect.w = Mediator::get_instance()->game_data.game_npcs[k].frame_size.width;
                Mediator::get_instance()->game_data.game_npcs[k].sprites[l][m].colision_rect.h = Mediator::get_instance()->game_data.game_npcs[k].frame_size.height;
			}
		}
	}
}


void assert_enum_items() {
	assert(PROJECTILE_TRAJECTORIES_NAMES.size() == PROJECTILE_TRAJECTORIES_COUNT);
	assert(AI_ACTION_NAMES.size() == AI_ACTION_LIST_SIZE);
}

// removes items, teleports and npcs that are placed out of the map for some reason
void clean_bad_data() {
    for (int i=0; i<MAX_STAGES; i++) {
        // clean links
        for (int j=0; j<STAGE_MAX_LINKS; j++) {
            if (Mediator::get_instance()->stage_data.stages[i].links[j].pos_origin.x < 0 || Mediator::get_instance()->stage_data.stages[i].links[j].pos_origin.x > MAP_W || Mediator::get_instance()->stage_data.stages[i].links[j].pos_origin.y < 0 || Mediator::get_instance()->stage_data.stages[i].links[j].pos_origin.y > MAP_H || Mediator::get_instance()->stage_data.stages[i].links[j].pos_destiny.x < 0 || Mediator::get_instance()->stage_data.stages[i].links[j].pos_destiny.x > MAP_W || Mediator::get_instance()->stage_data.stages[i].links[j].pos_destiny.y < 0 || Mediator::get_instance()->stage_data.stages[i].links[j].pos_destiny.y > MAP_H) {
                Mediator::get_instance()->stage_data.stages[i].links[j].id_map_destiny = -1;
                Mediator::get_instance()->stage_data.stages[i].links[j].id_map_origin = -1;
                Mediator::get_instance()->stage_data.stages[i].links[j].pos_origin.x = 0;
                Mediator::get_instance()->stage_data.stages[i].links[j].pos_origin.y = 0;
            }
        }
        // clean NPCs and Objects
        for (int j=0; j<STAGE_MAX_MAPS; j++) {
            for (int k=0; k<MAX_MAP_NPC_N; k++) {
                if (Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[k].start_point.x < 0 || Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[k].start_point.x > MAP_W || Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[k].start_point.y < 0 || Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[k].start_point.y > MAP_H) {
                    Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[k].start_point.x = 0;
                    Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[k].start_point.y = 0;
                    Mediator::get_instance()->stage_data.stages[i].maps[j].map_npcs[k].id_npc = -1;
                }
                if (Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[k].start_point.x < 0 || Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[k].start_point.x > MAP_W || Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[k].start_point.y < 0 || Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[k].start_point.y > MAP_H) {
                    Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[k].start_point.x = 0;
                    Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[k].start_point.y = 0;
                    Mediator::get_instance()->stage_data.stages[i].maps[j].map_objects[k].id_object = -1;
                }
            }
        }
    }
}


#undef main
int main(int argc, char *argv[])
{
	std::string EXEC_NAME;
	#ifndef WIN32
		EXEC_NAME = "editor";
    #else
		EXEC_NAME = "editor.exe";
	#endif

	std::string argvString = std::string(argv[0]);
    GAMEPATH = argvString.substr(0, argvString.size()-EXEC_NAME.size());
    std::cout << " *** EXEC_NAME: " << EXEC_NAME << ", FILEPATH: " << FILEPATH << ", SAVEPATH: " << SAVEPATH << " ***" << std::endl;

    FILEPATH = "";

    init_enum_names();
    assert_enum_items(); // check that stringfy variables are OK


    Mediator::get_instance()->fio.read_game(Mediator::get_instance()->game_data);
    Mediator::get_instance()->fio.read_all_stages(Mediator::get_instance()->stage_data);
    Mediator::get_instance()->fio.load_scene_sequence(Mediator::get_instance()->sequences);
    Mediator::get_instance()->fio.load_scenes(Mediator::get_instance()->scenes);
    clean_bad_data();

    QApplication a(argc, argv);

    MainWindow w;
    w.setWindowState(Qt::WindowMaximized);

    std::vector<std::string> game_list = Mediator::get_instance()->fio.read_game_list();

    if (game_list.size() < 1) {
        NewGameDialog *new_game_dialog = new NewGameDialog();
        QObject::connect(new_game_dialog, SIGNAL(on_accepted(QString)), &w, SLOT(on_new_game_accepted(QString)));
        new_game_dialog->show();
    } else if (game_list.size() == 1) {
        FILEPATH = GAMEPATH + std::string("/games/") + game_list.at(0) + std::string("/");
        GAMENAME = game_list.at(0);
        Mediator::get_instance()->loadGame();
        w.reload();
        w.show();
    } else {
        QDialog *open = new loadGamePicker();
        QObject::connect(open, SIGNAL(game_picked()), &w, SLOT(reload()));
        open->show();
    }

	remove_duplicated();

    return a.exec();
}
