#include "artificial_inteligence_tab.h"
#include "ui_artificial_inteligence_tab.h"
#include "common.h"
#include "mediator.h"

artificial_inteligence_tab::artificial_inteligence_tab(QWidget *parent) :
    QWidget(parent),
	ui(new Ui::artificial_inteligence_tab),
	_filling_data(true)
{
    ui->setupUi(this);
    fill_next_combos();
    fill_data(Mediator::get_instance()->current_ai);
}



artificial_inteligence_tab::~artificial_inteligence_tab()
{
    delete ui;
}

void artificial_inteligence_tab::reload()
{
    _filling_data = true;
    fill_next_combos();
    fill_data(Mediator::get_instance()->current_ai);
    _filling_data = false;
}



void artificial_inteligence_tab::on_ai_selector_currentIndexChanged(int index)
{
	if (_filling_data == true) {
		return;
	}
	_filling_data = true;

    // if no such AI exists in list yet, create it
    if (Mediator::get_instance()->enemy_list.size() > Mediator::get_instance()->ai_list.size()) {
        for (unsigned int i=Mediator::get_instance()->ai_list.size(); i<Mediator::get_instance()->enemy_list.size(); i++) {
            Mediator::get_instance()->ai_list.push_back(CURRENT_FILE_FORMAT::file_artificial_inteligence());
        }
    }

    Mediator::get_instance()->current_ai = index;

	// CHANCES
    ui->chance1->setValue(Mediator::get_instance()->ai_list.at(index).states[0].chance);
    ui->chance2->setValue(Mediator::get_instance()->ai_list.at(index).states[1].chance);
    ui->chance3->setValue(Mediator::get_instance()->ai_list.at(index).states[2].chance);
    ui->chance4->setValue(Mediator::get_instance()->ai_list.at(index).states[3].chance);
    ui->chance5->setValue(Mediator::get_instance()->ai_list.at(index).states[4].chance);
    ui->chance6->setValue(Mediator::get_instance()->ai_list.at(index).states[5].chance);
    ui->chance7->setValue(Mediator::get_instance()->ai_list.at(index).states[6].chance);
    ui->chance8->setValue(Mediator::get_instance()->ai_list.at(index).states[7].chance);

    // ACTIONS
    ui->action1->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[0].action);
    ui->action2->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[1].action);
    ui->action3->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[2].action);
    ui->action4->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[3].action);
    ui->action5->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[4].action);
    ui->action6->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[5].action);
    ui->action7->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[6].action);
    ui->action8->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[7].action);


    // REACTIONS //
    ui->aiReactionType_comboBox->setCurrentIndex(0);
    int action_n = Mediator::get_instance()->ai_list.at(index).reactions[0].action;
    ui->aiReactionAction_comboBox->setCurrentIndex(action_n+1); // plus 1 because of "none"
    common::fill_ai_options_combo(action_n, ui->aiReactionParam_comboBox); // plus one because of "none" option
    ui->aiReactionParam_comboBox->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).reactions[0].extra_parameter);

	// OPTIONS
    common::fill_ai_options_combo(Mediator::get_instance()->ai_list.at(index).states[0].action, ui->parameter1);
    common::fill_ai_options_combo(Mediator::get_instance()->ai_list.at(index).states[1].action, ui->parameter2);
    common::fill_ai_options_combo(Mediator::get_instance()->ai_list.at(index).states[2].action, ui->parameter3);
    common::fill_ai_options_combo(Mediator::get_instance()->ai_list.at(index).states[3].action, ui->parameter4);
    common::fill_ai_options_combo(Mediator::get_instance()->ai_list.at(index).states[4].action, ui->parameter5);
    common::fill_ai_options_combo(Mediator::get_instance()->ai_list.at(index).states[5].action, ui->parameter6);
    common::fill_ai_options_combo(Mediator::get_instance()->ai_list.at(index).states[6].action, ui->parameter7);
    common::fill_ai_options_combo(Mediator::get_instance()->ai_list.at(index).states[7].action, ui->parameter8);

    ui->parameter1->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[0].extra_parameter);
    ui->parameter2->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[1].extra_parameter);
    ui->parameter3->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[2].extra_parameter);
    ui->parameter4->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[3].extra_parameter);
    ui->parameter5->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[4].extra_parameter);
    ui->parameter6->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[5].extra_parameter);
    ui->parameter7->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[6].extra_parameter);
    ui->parameter8->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[7].extra_parameter);

	// go-tos
    ui->next1->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[0].go_to);
    ui->next2->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[1].go_to);
    ui->next3->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[2].go_to);
    ui->next4->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[3].go_to);
    ui->next5->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[4].go_to);
    ui->next6->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[5].go_to);
    ui->next7->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[6].go_to);
    ui->next8->setCurrentIndex(Mediator::get_instance()->ai_list.at(index).states[7].go_to);

	// delays
    ui->delay1->setValue(Mediator::get_instance()->ai_list.at(index).states[0].go_to_delay);
    ui->delay2->setValue(Mediator::get_instance()->ai_list.at(index).states[1].go_to_delay);
    ui->delay3->setValue(Mediator::get_instance()->ai_list.at(index).states[2].go_to_delay);
    ui->delay4->setValue(Mediator::get_instance()->ai_list.at(index).states[3].go_to_delay);
    ui->delay5->setValue(Mediator::get_instance()->ai_list.at(index).states[4].go_to_delay);
    ui->delay6->setValue(Mediator::get_instance()->ai_list.at(index).states[5].go_to_delay);
    ui->delay7->setValue(Mediator::get_instance()->ai_list.at(index).states[6].go_to_delay);
    ui->delay8->setValue(Mediator::get_instance()->ai_list.at(index).states[7].go_to_delay);

	_filling_data = false;
}

void artificial_inteligence_tab::fill_data(int index)
{
	// ai selector

    common::fill_npc_combo(ui->ai_selector);
    ui->ai_selector->setCurrentIndex(index);
    ui->aiReactionType_comboBox->setCurrentIndex(0);

	// actions
	common::fill_ai_actions_combo(ui->action1);
	common::fill_ai_actions_combo(ui->action2);
	common::fill_ai_actions_combo(ui->action3);
	common::fill_ai_actions_combo(ui->action4);
    common::fill_ai_actions_combo(ui->action5);
    common::fill_ai_actions_combo(ui->action6);
    common::fill_ai_actions_combo(ui->action7);
    common::fill_ai_actions_combo(ui->action8);
    common::fill_ai_actions_combo(ui->aiReactionAction_comboBox, true);

	_filling_data = false;
	on_ai_selector_currentIndexChanged(index);
}

void artificial_inteligence_tab::change_action(int index, int action_n)
{
    _filling_data = true;

    if (index >= 0) {
        Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[index].action = action_n;
        Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[index].extra_parameter = 0;
    // -1: near-player, -2: hit, -3: dead
    }

    if (index == 0) {
        common::fill_ai_options_combo(action_n, ui->parameter1);
    } else if (index == 1) {
        common::fill_ai_options_combo(action_n, ui->parameter2);
    } else if (index == 2) {
        common::fill_ai_options_combo(action_n, ui->parameter3);
    } else if (index == 3) {
        common::fill_ai_options_combo(action_n, ui->parameter4);
    } else if (index == 4) {
        common::fill_ai_options_combo(action_n, ui->parameter5);
    } else if (index == 5) {
        common::fill_ai_options_combo(action_n, ui->parameter6);
    } else if (index == 6) {
        common::fill_ai_options_combo(action_n, ui->parameter7);
    } else if (index == 7) {
        common::fill_ai_options_combo(action_n, ui->parameter8);
    }

    _filling_data = false;

}


void artificial_inteligence_tab::on_chance1_valueChanged(int arg1)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[0].chance = arg1;
}

void artificial_inteligence_tab::on_chance2_valueChanged(int arg1)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[1].chance = arg1;
}

void artificial_inteligence_tab::on_chance3_valueChanged(int arg1)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[2].chance = arg1;
}

void artificial_inteligence_tab::on_chance4_valueChanged(int arg1)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[3].chance = arg1;
}


void artificial_inteligence_tab::on_chance5_valueChanged(int arg1)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[4].chance = arg1;
}

void artificial_inteligence_tab::on_chance6_valueChanged(int arg1)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[5].chance = arg1;
}

void artificial_inteligence_tab::on_chance7_valueChanged(int arg1)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[6].chance = arg1;
}

void artificial_inteligence_tab::on_chance8_valueChanged(int arg1)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[7].chance = arg1;
}

void artificial_inteligence_tab::on_action1_currentIndexChanged(int index)
{
	if (_filling_data == true) {
		return;
	}
	change_action(0, index);
}

void artificial_inteligence_tab::on_action2_currentIndexChanged(int index)
{
	if (_filling_data == true) {
		return;
	}
	change_action(1, index);
}

void artificial_inteligence_tab::on_action3_currentIndexChanged(int index)
{
	if (_filling_data == true) {
		return;
	}
	change_action(2, index);
}

void artificial_inteligence_tab::on_action4_currentIndexChanged(int index)
{
	if (_filling_data == true) {
		return;
	}
	change_action(3, index);
}

void artificial_inteligence_tab::on_action5_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    change_action(4, index);
}


void artificial_inteligence_tab::on_action6_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    change_action(5, index);
}

void artificial_inteligence_tab::on_action7_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    change_action(6, index);
}

void artificial_inteligence_tab::on_action8_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    change_action(7, index);
}

void artificial_inteligence_tab::on_parameter1_currentIndexChanged(int index)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[0].extra_parameter = index;
}

void artificial_inteligence_tab::on_parameter2_currentIndexChanged(int index)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[1].extra_parameter = index;

}

void artificial_inteligence_tab::on_parameter3_currentIndexChanged(int index)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[2].extra_parameter = index;
}

void artificial_inteligence_tab::on_parameter4_currentIndexChanged(int index)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[3].extra_parameter = index;
}


void artificial_inteligence_tab::on_parameter5_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[4].extra_parameter = index;
}

void artificial_inteligence_tab::on_parameter6_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[5].extra_parameter = index;
}

void artificial_inteligence_tab::on_parameter7_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[6].extra_parameter = index;
}

void artificial_inteligence_tab::on_parameter8_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[7].extra_parameter = index;
}


void artificial_inteligence_tab::on_next1_currentIndexChanged(int index)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[0].go_to = index;
}

void artificial_inteligence_tab::on_next2_currentIndexChanged(int index)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[1].go_to = index;
}

void artificial_inteligence_tab::on_next3_currentIndexChanged(int index)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[2].go_to = index;
}

void artificial_inteligence_tab::on_next4_currentIndexChanged(int index)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[3].go_to = index;
}




void artificial_inteligence_tab::on_next5_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[4].go_to = index;

}

void artificial_inteligence_tab::on_next6_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[5].go_to = index;

}

void artificial_inteligence_tab::on_next7_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[6].go_to = index;

}

void artificial_inteligence_tab::on_next8_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[7].go_to = index;

}


void artificial_inteligence_tab::on_delay1_valueChanged(int arg1)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[0].go_to_delay = arg1;
}

void artificial_inteligence_tab::on_delay2_valueChanged(int arg1)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[1].go_to_delay = arg1;
}

void artificial_inteligence_tab::on_delay3_valueChanged(int arg1)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[2].go_to_delay = arg1;
}

void artificial_inteligence_tab::on_delay4_valueChanged(int arg1)
{
	if (_filling_data == true) {
		return;
	}
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[3].go_to_delay = arg1;
}


void artificial_inteligence_tab::on_delay5_valueChanged(int arg1)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[4].go_to_delay = arg1;

}

void artificial_inteligence_tab::on_delay6_valueChanged(int arg1)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[5].go_to_delay = arg1;

}

void artificial_inteligence_tab::on_delay7_valueChanged(int arg1)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[6].go_to_delay = arg1;

}

void artificial_inteligence_tab::on_delay8_valueChanged(int arg1)
{
    if (_filling_data == true) {
        return;
    }
    Mediator::get_instance()->ai_list.at(Mediator::get_instance()->current_ai).states[7].go_to_delay = arg1;

}

void artificial_inteligence_tab::fill_next_combos()
{
    fill_next_combo(ui->next1);
    fill_next_combo(ui->next2);
    fill_next_combo(ui->next3);
    fill_next_combo(ui->next4);
    fill_next_combo(ui->next5);
    fill_next_combo(ui->next6);
    fill_next_combo(ui->next7);
    fill_next_combo(ui->next8);
}

void artificial_inteligence_tab::fill_next_combo(QComboBox *combo)
{
    combo->clear(); // delete all previous entries
    std::vector<std::string> list;
    list.push_back("CHANCE");
    list.push_back("#1");
    list.push_back("#2");
    list.push_back("#3");
    list.push_back("#4");
    list.push_back("#5");
    list.push_back("#6");
    list.push_back("#7");
    list.push_back("#8");
    for (unsigned int i=0; i<list.size(); i++) {
        std::string temp = list[i];
        QString temp_str = QString("[") + QString::number(i) + QString("] - ") + QString(temp.c_str());
        combo->addItem(temp_str);
    }
}




void artificial_inteligence_tab::on_aiReactionAction_comboBox_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }

    _filling_data = true;
    std::cout << "reaction current index #1 [" << ui->aiReactionType_comboBox->currentIndex() << "]" << std::endl;
    Mediator::get_instance()->ai_list.at(ui->ai_selector->currentIndex()).reactions[ui->aiReactionType_comboBox->currentIndex()].action = index-1; // less one because of "none"
    common::fill_ai_options_combo(index-1, ui->aiReactionParam_comboBox);

    _filling_data = false;
}

void artificial_inteligence_tab::on_aiReactionParam_comboBox_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    std::cout << "reaction current index #2 [" << ui->aiReactionType_comboBox->currentIndex() << "]" << std::endl;
    Mediator::get_instance()->ai_list.at(ui->ai_selector->currentIndex()).reactions[ui->aiReactionType_comboBox->currentIndex()].extra_parameter = index;
}

void artificial_inteligence_tab::on_aiReactionType_comboBox_currentIndexChanged(int index)
{
    if (_filling_data == true) {
        return;
    }
    std::cout << "reaction current index #3 [" << ui->aiReactionType_comboBox->currentIndex() << "]" << std::endl;

    _filling_data = true;
    int action_n = Mediator::get_instance()->ai_list.at(ui->ai_selector->currentIndex()).reactions[index].action;
    ui->aiReactionAction_comboBox->setCurrentIndex(action_n+1); // plus 1 because of "none"
    common::fill_ai_options_combo(action_n, ui->aiReactionParam_comboBox); // plus one because of "none" option
    ui->aiReactionParam_comboBox->setCurrentIndex(Mediator::get_instance()->ai_list.at(ui->ai_selector->currentIndex()).reactions[index].extra_parameter);
    _filling_data = false;
}
