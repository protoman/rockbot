#include "common.h"

extern std::vector<std::string> PROJECTILE_TRAJECTORIES_NAMES;
extern std::vector<std::string> AI_ACTION_NAMES;
extern std::vector<std::string> AI_ACTION_WALK_OPTIONS;
extern std::vector<std::string> AI_ACTION_SHOT_OPTIONS;
extern std::vector<std::string> AI_ACTION_FLY_OPTIONS;
extern std::vector<std::string> AI_ACTION_JUMP_OPTIONS;
extern std::vector<std::string> AI_ACTION_AIR_WALK_OPTIONS;
extern std::vector<std::string> AI_ACTION_TELEPORT_OPTIONS;
extern std::vector<std::string> AI_ACTION_DASH_OPTIONS;
extern std::vector<std::string> AI_ACTION_GRAB_WALL_OPTIONS;
extern std::vector<std::string> ANIMATION_TYPE_LIST;

#include "mediator.h"

common::common()
{
}

common::~common()
{
}


void common::fill_files_combo(std::string directory, QComboBox* combo, bool show_none)
{
    // ignore as game was not initialized yet. the form will need to call reload to work in this case
    if (FILEPATH.length() == 0) {
        return;
    }

    combo->clear(); // delete all previous entries

    combo->addItem(QString("")); // for "empty"
    std::string str_filepath(FILEPATH+directory);
    QString filepath(str_filepath.c_str());
    QDir dir = QDir(filepath);

    if (!dir.exists()) {
        std::cout << ">> MainWindow::fill_files_combo ERROR: Directory '" << str_filepath << " does not exist. <<" << std::endl;
        exit(-1);
    }

    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
	if (show_none == true) {
		combo->addItem(QString("None"));
	}

    foreach (const QFileInfo &fileInfo, dir.entryInfoList()) {
        if (fileInfo.fileName().length() > 30) {
            std::cout << "ERROR: file '" << fileInfo.fileName().toStdString() << "' surpasses the maximum number of file-characters (" << FS_CHAR_NAME_SIZE << ")" << std::endl;
        } else {
            QString filename(fileInfo.fileName());
            if (filename.length() > 0) {
                combo->addItem(filename);
            }
        }
    }
    combo->repaint();
}

void common::fill_graphicfiles_listwidget(std::string directory, QListWidget* listWidget)
{
    listWidget->clear();

    QListWidgetItem* item;

    std::string str_filepath(FILEPATH+directory);
    QString filepath(str_filepath.c_str());
    QDir dir = QDir(filepath);
    if (!dir.exists()) {
        std::cout << ">> MainWindow::fill_graphicfiles_listwidget ERROR: Directory '" << str_filepath << " does not exist. <<" << std::endl;
        exit(-1);
    }
    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Size | QDir::Reversed);
    QFileInfoList list = dir.entryInfoList();

    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.fileName().length() > 30) {
            std::cout << "ERROR: file '" << fileInfo.fileName().toStdString() << "' surpasses the maximum number of file-characters (" << FS_CHAR_NAME_SIZE << ")" << std::endl;
        } else {
            item = new QListWidgetItem;
            item->setText(fileInfo.fileName());
            std::string filename = FILEPATH + directory + "/" + fileInfo.fileName().toStdString();
            //std::cout << ">> MainWindow::fill_graphicfiles_listwidget DEBUG: filename: '" << filename << std::endl;
            QPixmap image(filename.c_str());
            if (image.isNull() == false && image.width() > 0) {
                image = image.copy(0, 0, image.width(), image.height());
                image = image.scaled(32, 32);
                item->setIcon(image);
            }
            listWidget->addItem(item);
        }
    }
    listWidget->repaint();
}

void common::fill_graphicfiles_combobox(std::string directory, QComboBox *comboWidget)
{
    comboWidget->clear(); // delete all previous entries

    std::string str_filepath(FILEPATH+directory);
    QString filepath(str_filepath.c_str());
    QDir dir = QDir(filepath);
    if (!dir.exists()) {
        std::cout << ">> MainWindow::fill_graphicfiles_combobox ERROR: Directory '" << str_filepath << " does not exist. <<" << std::endl;
        exit(-1);
    }
    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Size | QDir::Reversed);
    QFileInfoList list = dir.entryInfoList();

    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.fileName().length() > 30) {
            std::cout << "ERROR: file '" << fileInfo.fileName().toStdString() << "' surpasses the maximum number of file-characters (" << FS_CHAR_NAME_SIZE << ")" << std::endl;
        } else {
            std::string filename = FILEPATH + directory + "/" + fileInfo.fileName().toStdString();
            QIcon icon(filename.c_str());
            comboWidget->addItem(icon, fileInfo.fileName());
        }
    }
    comboWidget->repaint();
}


void common::fill_npc_combo(QComboBox* combo)
{
    combo->clear(); // delete all previous entries

    for (int i=0; i<Mediator::get_instance()->enemy_list.size(); i++) {
        QString temp_str = QString("[");
        if (i < 10) {
            temp_str += "0";
        }
        temp_str += QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->enemy_list.at(i).name);
        combo->addItem(temp_str);
    }
}

void common::fill_object_combo(QComboBox* combo)
{
    combo->clear(); // delete all previous entries

    for (int i=0; i<Mediator::get_instance()->object_list.size(); i++) {
        QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->object_list.at(i).name);
        combo->addItem(temp_str);
	}
}

// weapons combo is filled with stages 1-9 (5 is skullcastle and do not count)
void common::fill_weapons_combo(QComboBox *combo)
{
    combo->clear(); // delete all previous entries

    QString temp_str = QString("[0] - Normal Weapon");
    combo->addItem(temp_str);
    for (int i=1; i<9; i++) {
        temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->stage_data.stages[i].name + QString(" (") + QString(Mediator::get_instance()->stage_data.stages[i].boss.name) + QString (")"));
        combo->addItem(temp_str);
    }
}

void common::fill_weapons_names_combo(QComboBox *combo)
{
    combo->clear(); // delete all previous entries
    for (int i=0; i<FS_MAX_WEAPONS; i++) {
        QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->game_data.weapons[i].name);
        combo->addItem(temp_str);
    }
}

// same as fill_weapons_combo(), but also add jet and coil items
void common::fill_weapons_combo_plus(QComboBox *combo)
{
    fill_weapons_combo(combo);
    QString temp_str = QString(QString("[9] - Coil Item"));
    combo->addItem(temp_str);
    temp_str = QString(QString("[10] - Jet Item"));
    combo->addItem(temp_str);
}

void common::fill_projectiles_combo(QComboBox *combo, bool add_empty_slot)
{
    combo->clear(); // delete all previous entries

    if (add_empty_slot) {
        combo->addItem(QString(""));
    }
    for (int i=0; i<Mediator::get_instance()->projectile_list_v2.size(); i++) {
        QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->projectile_list_v2.at(i).name);
        combo->addItem(temp_str);
    }
}


void common::fill_trajectories_combo(QComboBox *combo)
{
    combo->clear(); // delete all previous entries

    for (int i=0; i<PROJECTILE_TRAJECTORIES_COUNT; i++) {
		std::string temp = PROJECTILE_TRAJECTORIES_NAMES[i];
		QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(temp.c_str());
		combo->addItem(temp_str);
	}
}

void common::fill_ai_actions_combo(QComboBox *combo, bool include_null)
{
    combo->clear(); // delete all previous entries
    if (include_null == true) {
        QString temp_null_str = QString("None");
        combo->addItem(temp_null_str);
    }

    for (int i=0; i<AI_ACTION_NAMES.size(); i++) {
		std::string temp = AI_ACTION_NAMES[i];
		QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(temp.c_str());
		combo->addItem(temp_str);
	}
}


void common::fill_ai_options_combo(int action, QComboBox *combo)
{

    combo->clear(); // delete all previous entries

    //action--;

    std::vector<std::string> list;
	if (action == AI_ACTION_WALK) {
		list = AI_ACTION_WALK_OPTIONS;
	} else if (action == AI_ACTION_FLY) {
		list = AI_ACTION_FLY_OPTIONS;
	} else if (action == AI_ACTION_JUMP) {
		list = AI_ACTION_JUMP_OPTIONS;
	} else if (action == AI_ACTION_AIR_WALK) {
		list = AI_ACTION_AIR_WALK_OPTIONS;
	} else if (action == AI_ACTION_TELEPORT) {
		list = AI_ACTION_TELEPORT_OPTIONS;
	} else if (action == AI_ACTION_DASH) {
		list = AI_ACTION_DASH_OPTIONS;
	} else if (action == AI_ACTION_GRAB_WALL) {
		list = AI_ACTION_GRAB_WALL_OPTIONS;
    } else if (action == AI_ACTION_SPAWN_NPC || action == AI_ACTION_REPLACE_NPC) {
        list = common::get_npc_names_list();
    } else if (action == AI_ACTION_SHOT_PROJECTILE_AHEAD || action == AI_ACTION_SHOT_PROJECTILE_PLAYER_DIRECTION || action == AI_ACTION_SHOT_PROJECTILE_INVERT_DIRECTION || action == AI_ACTION_JUMP_ATTACK_UP || action == AI_ACTION_JUMP_ATTACK_AHEAD_ONCE) {
        //list = AI_ACTION_SHOT_OPTIONS;
        list = common::get_weapon_names_list();
    } else if (action == AI_ACTION_CHANGE_MOVE_TYPE || action == AI_ACTION_CHANGE_MOVE_TYPE_REVERSE) {
        list = ANIMATION_TYPE_LIST;
    } else if (action == AI_ACTION_CIRCLE_PLAYER) {
        std::vector<std::string> dist_list;
        dist_list.push_back("1");
        dist_list.push_back("2");
        dist_list.push_back("3");
        dist_list.push_back("4");
        list = dist_list;
    } else if (action == AI_ACTION_WAIT_RANDOM_TIME) {
        std::vector<std::string> dist_list;
        dist_list.push_back("1");
        dist_list.push_back("2");
        dist_list.push_back("3");
        dist_list.push_back("4");
        dist_list.push_back("5");
        dist_list.push_back("10");
        dist_list.push_back("20");
        list = dist_list;
    } else {
        return;
    }

	// add options
    combo->clear(); // delete all previous entries
	for (int i=0; i<list.size(); i++) {
		std::string temp = list[i];
		QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(temp.c_str());
		combo->addItem(temp_str);
	}
}

void common::fill_ai_list(QComboBox *combo)
{
    combo->clear(); // delete all previous entries
    for (int i=0; i<Mediator::get_instance()->ai_list.size(); i++) {
        QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->ai_list.at(i).name);
		combo->addItem(temp_str);
    }
}

void common::fill_stages_combo(QComboBox *combo)
{
    combo->clear(); // delete all previous entries
    for (int i=0; i<FS_MAX_STAGES; i++) {
        combo->addItem(QString("[") + QString::number(i) + QString("]: ") + QString(Mediator::get_instance()->stage_data.stages[i].name));
    }
}

void common::fill_players_combo(QComboBox* combo)
{
    combo->clear(); // delete all previous entries
    for (int i=0; i<FS_MAX_PLAYERS; i++) {
        combo->addItem(QString::number(i+1)+QString(" [")+QString(Mediator::get_instance()->player_list[i].name)+QString("]"));
    }
}


void common::fill_map_list_combo(QComboBox *combo)
{
    combo->clear(); // delete all previous entries
    for (int i=0; i<3; i++) {
        char buffer[512];
        Mediator::get_instance()->centNumberFormat(i);
        sprintf(buffer, "%s", Mediator::get_instance()->centNumber);
        combo->addItem(buffer);
    }
}


void common::fill_npc_listwidget(QListWidget *listWidget)
{
    QListWidgetItem* item;

    listWidget->clear();

    for (int i=0; i<Mediator::get_instance()->enemy_list.size(); i++) {
        item = new QListWidgetItem;
        QString temp_str = QString("[");
        if (i < 10) {
            temp_str += "0";
        }
        temp_str += QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->enemy_list.at(i).name);
        item->setText(temp_str);
        std::string filename = FILEPATH + "/images/sprites/enemies/" + Mediator::get_instance()->enemy_list.at(i).graphic_filename;
        QPixmap image(filename.c_str());
        image = image.copy(0, 0, Mediator::get_instance()->enemy_list.at(i).frame_size.width, Mediator::get_instance()->enemy_list.at(i).frame_size.height);
        if (image.isNull() == false && image.width() > 0) {
            image = image.scaled(32, 32);
        }
        item->setIcon(image);
        listWidget->addItem(item);
    }

    listWidget->update();
}

void common::fill_object_listWidget(QListWidget *listWidget)
{
    QListWidgetItem* item;

    listWidget->clear();

    for (int i=0; i<Mediator::get_instance()->object_list.size(); i++) {
        item = new QListWidgetItem;
        QString temp_str = QString("[");
        if (i < 10) {
            temp_str += "0";
        }

        temp_str += QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->object_list.at(i).name);
        item->setText(temp_str);
        std::string filename = FILEPATH + "/images/sprites/objects/" + Mediator::get_instance()->object_list.at(i).graphic_filename;
        QPixmap image(filename.c_str());
        image = image.copy(0, 0, Mediator::get_instance()->object_list.at(i).size.width, Mediator::get_instance()->object_list.at(i).size.height);
        if (image.isNull() == false && image.width() > 0) {
            image = image.scaled(32, 32);
        }
        item->setIcon(image);
        listWidget->addItem(item);
    }


    listWidget->update();
}

void common::fill_scenes_combo(QComboBox *combo)
{
    combo->clear();
    // add empty item to be used to unset scene
    combo->addItem(QString(""));
    for (int i=0; i<Mediator::get_instance()->scene_list.size(); i++) {
        QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->scene_list.at(i).name);
        combo->addItem(temp_str);
    }
}

std::vector<std::string> common::get_npc_names_list()
{
    std::vector<std::string> res;
    for (int i=0; i<Mediator::get_instance()->enemy_list.size(); i++) {
        res.push_back(std::string(Mediator::get_instance()->enemy_list.at(i).name));
    }
    return res;
}

std::vector<std::string> common::get_weapon_names_list()
{
    std::vector<std::string> res;
    for (int i=0; i<Mediator::get_instance()->projectile_list_v2.size(); i++) {
        res.push_back(std::string(Mediator::get_instance()->projectile_list_v2.at(i).name));
    }
    return res;
}

st_size common::calc_image_size(std::string file)
{
    std::string filename = FILEPATH + std::string("/images/scenes/animations/") + file;
    QPixmap image(filename.c_str());

    if (image.isNull() == true || image.width() <= 0) {
        return st_size(0, 0);
    }
    return st_size(image.width(), image.height());
}

void common::fill_anim_block_combo(QComboBox *combo)
{
    combo->clear();
    for (int i=0; i<Mediator::get_instance()->anim_block_list.size(); i++) {
        QString name = QString::number(i);
        combo->addItem(name);
    }
}




