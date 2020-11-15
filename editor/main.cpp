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

bool GAME_FLAGS[FLAG_COUNT];

void remove_duplicated()
{

}

void adjust_sprites_size() {
    for (int k=0; k<Mediator::get_instance()->enemy_list.size(); k++) {
		for (int l=0; l<ANIM_TYPE_COUNT; l++) {
			for (int m=0; m<ANIM_FRAMES_COUNT; m++) {
                Mediator::get_instance()->enemy_list.at(k).sprites[l][m].collision_rect.x = 0;
                Mediator::get_instance()->enemy_list.at(k).sprites[l][m].collision_rect.y = 0;
                Mediator::get_instance()->enemy_list.at(k).sprites[l][m].collision_rect.w = Mediator::get_instance()->enemy_list.at(k).frame_size.width;
                Mediator::get_instance()->enemy_list.at(k).sprites[l][m].collision_rect.h = Mediator::get_instance()->enemy_list.at(k).frame_size.height;
			}
		}
	}
}


void assert_enum_items() {
	assert(PROJECTILE_TRAJECTORIES_NAMES.size() == PROJECTILE_TRAJECTORIES_COUNT);
	assert(AI_ACTION_NAMES.size() == AI_ACTION_LIST_SIZE);
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

    FILEPATH = "";

    init_enum_names();
    assert_enum_items(); // check that stringfy variables are OK


    QApplication a(argc, argv);

    std::vector<std::string> game_list = Mediator::get_instance()->fio.read_game_list();

    MainWindow w;
    w.setWindowState(Qt::WindowMaximized);


    if (game_list.size() < 1) {
        NewGameDialog *new_game_dialog = new NewGameDialog();
        QObject::connect(new_game_dialog, SIGNAL(on_accepted(QString)), &w, SLOT(on_new_game_accepted(QString)));
        new_game_dialog->show();
    } else if (game_list.size() == 1) {
        FILEPATH = GAMEPATH + std::string("/games/") + game_list.at(0) + std::string("/");
        GAMENAME = game_list.at(0);
        Mediator::get_instance()->load_game();
        w.reload();
        w.show();
    } else {
        QDialog *open = new loadGamePicker();
        QObject::connect(open, SIGNAL(game_picked()), &w, SLOT(on_load_game_accepted()));
        open->show();
    }

	remove_duplicated();

    return a.exec();
}
