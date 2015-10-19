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

#include "mediator.h"

common::common()
{
}

common::~common()
{
}


void common::fill_files_combo(std::string directory, QComboBox* combo, bool show_none)
{

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
    dir.setSorting(QDir::Size | QDir::Reversed);
	if (show_none == true) {
		combo->addItem(QString("None"));
	}

    foreach (const QFileInfo &fileInfo, dir.entryInfoList()) {
        if (fileInfo.fileName().length() > 30) {
            std::cout << "ERROR: file '" << fileInfo.fileName().toStdString() << "' surpasses the maximum number of file-characters (" << CHAR_FILENAME_SIZE << ")" << std::endl;
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
            std::cout << "ERROR: file '" << fileInfo.fileName().toStdString() << "' surpasses the maximum number of file-characters (" << CHAR_FILENAME_SIZE << ")" << std::endl;
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
            std::cout << "ERROR: file '" << fileInfo.fileName().toStdString() << "' surpasses the maximum number of file-characters (" << CHAR_FILENAME_SIZE << ")" << std::endl;
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

    for (int i=0; i<FS_GAME_MAX_NPCS; i++) {
        QString temp_str = QString("[");
        if (i < 10) {
            temp_str += "0";
        }
        temp_str += QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->game_data.game_npcs[i].name);
        combo->addItem(temp_str);
    }
}

void common::fill_object_combo(QComboBox* combo)
{
    combo->clear(); // delete all previous entries

    for (int i=0; i<FS_GAME_MAX_NPCS; i++) {
        QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->game_data.objects[i].name);
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

void common::fill_projectiles_combo(QComboBox *combo)
{
    combo->clear(); // delete all previous entries

    combo->addItem(QString("[") + QString::number(-1) + QString("] - ") + QString("DEFAULT"));
    for (int i=0; i<FS_MAX_PROJECTILES; i++) {
        QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->game_data.projectiles[i].name);
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
    } else if (action == AI_ACTION_SPAWN_NPC) {
        list = common::get_npc_names_list();
    } else if (action == AI_ACTION_SHOT_PROJECTILE_1 || action == AI_ACTION_SHOT_PROJECTILE_2) {
        list = AI_ACTION_SHOT_OPTIONS;
    }

	// add options
	for (int i=0; i<list.size(); i++) {
		std::string temp = list[i];
		QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(temp.c_str());
		combo->addItem(temp_str);
	}
}

void common::fill_ai_list(QComboBox *combo)
{
    combo->clear(); // delete all previous entries
    for (int i=0; i<MAX_AI_TYPES; i++) {
        QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->game_data.ai_types[i].name);
		combo->addItem(temp_str);
    }
}

void common::fill_stages_combo(QComboBox *combo)
{
    combo->clear(); // delete all previous entries
    for (int i=0; i<FS_MAX_STAGES; i++) {
        if (strlen(Mediator::get_instance()->stage_data.stages[i].name) > 0) {
            //std::cout << "STAGE: '" << stage_data.stages[i].name << "'" << std::endl;
            combo->addItem(QString(Mediator::get_instance()->stage_data.stages[i].name));
        }
    }
}

void common::fill_players_combo(QComboBox* combo)
{
    combo->clear(); // delete all previous entries
    for (int i=0; i<FS_MAX_PLAYERS; i++) {
        combo->addItem(QString::number(i+1)+QString(" [")+QString(Mediator::get_instance()->game_data.players[i].name)+QString("]"));
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

    for (int i=0; i<FS_GAME_MAX_NPCS; i++) {
        item = new QListWidgetItem;
        QString temp_str = QString("[");
        if (i < 10) {
            temp_str += "0";
        }
        temp_str += QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->game_data.game_npcs[i].name);
        item->setText(temp_str);
        std::string filename = FILEPATH + "/images/sprites/enemies/" + Mediator::get_instance()->game_data.game_npcs[i].graphic_filename;
        QPixmap image(filename.c_str());
        image = image.copy(0, 0, Mediator::get_instance()->game_data.game_npcs[i].frame_size.width, Mediator::get_instance()->game_data.game_npcs[i].frame_size.height);
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

    for (int i=0; i<FS_GAME_MAX_NPCS; i++) {
        item = new QListWidgetItem;
        QString temp_str = QString("[");
        if (i < 10) {
            temp_str += "0";
        }
        temp_str += QString::number(i) + QString("] - ") + QString(Mediator::get_instance()->game_data.objects[i].name);
        item->setText(temp_str);
        std::string filename = FILEPATH + "/images/sprites/objects/" + Mediator::get_instance()->game_data.objects[i].graphic_filename;
        QPixmap image(filename.c_str());
        image = image.copy(0, 0, Mediator::get_instance()->game_data.objects[i].size.width, Mediator::get_instance()->game_data.objects[i].size.height);
        if (image.isNull() == false && image.width() > 0) {
            image = image.scaled(32, 32);
        }
        item->setIcon(image);
        listWidget->addItem(item);
    }


    listWidget->update();
}

std::vector<std::string> common::get_npc_names_list()
{
    std::vector<std::string> res;
    for (int i=0; i<FS_GAME_MAX_NPCS; i++) {
        res.push_back(std::string(Mediator::get_instance()->game_data.game_npcs[i].name));
    }
    return res;
}


