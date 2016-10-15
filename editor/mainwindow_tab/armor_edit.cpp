#include "armor_edit.h"
#include "common.h"

armor_edit::armor_edit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::armor_edit)
{
    ui->setupUi(this);
    _data_loading = true;
    fill_armor_abilities();
    _data_loading = false;
}

armor_edit::~armor_edit()
{
    delete ui;
}

void armor_edit::reload()
{
    // === ARMS === //
    ui->arm_player1_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[0][0]).c_str()));
    ui->arm_player1_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[0][1]).c_str()));
    ui->arm_player1_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[0][2]).c_str()));

    ui->arm_player2_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[1][0]).c_str()));
    ui->arm_player2_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[1][1]).c_str()));
    ui->arm_player2_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[1][2]).c_str()));

    ui->arm_player3_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[2][0]).c_str()));
    ui->arm_player3_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[2][1]).c_str()));
    ui->arm_player3_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[2][2]).c_str()));

    ui->arm_player4_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[3][0]).c_str()));
    ui->arm_player4_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[3][1]).c_str()));
    ui->arm_player4_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[3][2]).c_str()));

    // === BODY === //
    ui->body_player1_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[0][0]).c_str()));
    ui->body_player1_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[0][1]).c_str()));
    ui->body_player1_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[0][2]).c_str()));

    ui->body_player2_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[1][0]).c_str()));
    ui->body_player2_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[1][1]).c_str()));
    ui->body_player2_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[1][2]).c_str()));

    ui->body_player3_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[2][0]).c_str()));
    ui->body_player3_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[2][1]).c_str()));
    ui->body_player3_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[2][2]).c_str()));

    ui->body_player4_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[3][0]).c_str()));
    ui->body_player4_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[3][1]).c_str()));
    ui->body_player4_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[3][2]).c_str()));

    // === LEGS === //
    ui->legs_player1_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[0][0]).c_str()));
    ui->legs_player1_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[0][1]).c_str()));
    ui->legs_player1_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[0][2]).c_str()));

    ui->legs_player2_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[1][0]).c_str()));
    ui->legs_player2_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[1][1]).c_str()));
    ui->legs_player2_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[1][2]).c_str()));

    ui->legs_player3_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[2][0]).c_str()));
    ui->legs_player3_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[2][1]).c_str()));
    ui->legs_player3_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[2][2]).c_str()));

    ui->legs_player4_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[3][0]).c_str()));
    ui->legs_player4_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[3][1]).c_str()));
    ui->legs_player4_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[3][2]).c_str()));


    _data_loading = true;

    ui->p1_legs_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].special_ability[0]);
    ui->p1_body_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].special_ability[0]);

    ui->p2_legs_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].special_ability[1]);
    ui->p2_body_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].special_ability[1]);

    ui->p3_legs_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].special_ability[2]);
    ui->p3_body_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].special_ability[2]);

    ui->p4_legs_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].special_ability[3]);
    ui->p4_body_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].special_ability[3]);


    common::fill_projectiles_combo(ui->p1_arm_comboBox, true);
    ui->p1_arm_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].special_ability[0]+1);

    common::fill_projectiles_combo(ui->p2_arm_comboBox, true);
    ui->p2_arm_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].special_ability[1]+1);

    common::fill_projectiles_combo(ui->p3_arm_comboBox, true);
    ui->p3_arm_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].special_ability[2]+1);

    common::fill_projectiles_combo(ui->p4_arm_comboBox, true);
    ui->p4_arm_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].special_ability[3]+1);
    _data_loading = false;

}

void armor_edit::fill_armor_abilities()
{
    std::string legs_abilities[] = {"Double Jump", "Air-Dash", "Shoryuken"};
    std::string body_abilities[] = {"Half-Damage", "Extended Immunity", "Spikes Immune", "No Push-Back"};


    for (unsigned int i = 0; i<4; i++) {
        ui->p1_body_comboBox->addItem(body_abilities[i].c_str());
        ui->p2_body_comboBox->addItem(body_abilities[i].c_str());
        ui->p3_body_comboBox->addItem(body_abilities[i].c_str());
        ui->p4_body_comboBox->addItem(body_abilities[i].c_str());
    }

    for (unsigned int i = 0; i<3; i++) {
        ui->p1_legs_comboBox->addItem(legs_abilities[i].c_str());
        ui->p2_legs_comboBox->addItem(legs_abilities[i].c_str());
        ui->p3_legs_comboBox->addItem(legs_abilities[i].c_str());
        ui->p4_legs_comboBox->addItem(legs_abilities[i].c_str());
    }

    reload();
}


void armor_edit::on_p1_arm_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].special_ability[0] = index-1; // minus one because first one is empty (-1)
}

void armor_edit::on_p1_body_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].special_ability[0] = index;
}

void armor_edit::on_p1_legs_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].special_ability[0] = index;
}

void armor_edit::on_p2_arm_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].special_ability[1] = index-1; // minus one because first one is empty (-1)
}

void armor_edit::on_p2_body_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].special_ability[1] = index;
}

void armor_edit::on_p2_legs_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].special_ability[1] = index;
}

void armor_edit::on_p3_arm_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].special_ability[2] = index-1;  // minus one because first one is empty (-1)
}

void armor_edit::on_p3_body_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].special_ability[2] = index;
}

void armor_edit::on_p3_legs_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].special_ability[2] = index;
}

void armor_edit::on_p4_arm_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    std::cout << "on_p4_arm_comboBox_currentIndexChanged - index: " << index << std::endl;
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].special_ability[3] = index-1;  // minus one because first one is empty (-1)
}

void armor_edit::on_p4_body_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    std::cout << "on_p4_body_comboBox_currentIndexChanged - index: " << index << std::endl;
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].special_ability[3] = index;
}

void armor_edit::on_p4_legs_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) { return; }
    std::cout << "on_p4_legs_comboBox_currentIndexChanged - index: " << index << std::endl;
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].special_ability[3] = index;
}

void armor_edit::on_string_selected(int string_id)
{
    StringsEditor* strings_editor_window = (StringsEditor*)sender();
    int* value_property = strings_editor_window->get_target_property();
    *value_property = string_id;
    QLineEdit* qline = strings_editor_window->get_target_qline();
    qline->setText(QString(fio_str.get_common_string(string_id).c_str()));
}


void armor_edit::string_tooltip_click(int *property, QLineEdit *qline)
{
    StringsEditor* strings_editor_window = new StringsEditor(this, 1);
    QObject::connect(strings_editor_window, SIGNAL(on_accepted(int)), this, SLOT(on_string_selected(int)));
    strings_editor_window->set_target_property(property);
    strings_editor_window->set_target_qline(qline);
    strings_editor_window->show();
}

void armor_edit::on_arm_player1_toolButton1_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[0][0]), ui->arm_player1_text1);
}

void armor_edit::on_arm_player1_toolButton2_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[0][1]), ui->arm_player1_text2);
}

void armor_edit::on_arm_player1_toolButton3_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[0][2]), ui->arm_player1_text3);
}

void armor_edit::on_arm_player2_toolButton1_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[1][0]), ui->arm_player2_text1);
}

void armor_edit::on_arm_player2_toolButton2_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[1][1]), ui->arm_player2_text2);
}

void armor_edit::on_arm_player2_toolButton3_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[1][3]), ui->arm_player2_text3);
}

void armor_edit::on_arm_player3_toolButton1_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[2][0]), ui->arm_player3_text1);
}

void armor_edit::on_arm_player3_toolButton2_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[2][1]), ui->arm_player3_text2);
}

void armor_edit::on_arm_player3_toolButton3_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[2][2]), ui->arm_player3_text3);
}

void armor_edit::on_arm_player4_toolButton1_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[3][0]), ui->arm_player4_text1);
}

void armor_edit::on_arm_player4_toolButton2_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[3][1]), ui->arm_player4_text2);
}

void armor_edit::on_arm_player4_toolButton3_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_ARMS].got_message[3][2]), ui->arm_player4_text3);
}

void armor_edit::on_body_player1_toolButton1_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[0][0]), ui->body_player1_text1);
}

void armor_edit::on_body_player1_toolButton2_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[0][1]), ui->body_player1_text2);
}

void armor_edit::on_body_player1_toolButton3_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[0][2]), ui->body_player1_text3);
}

void armor_edit::on_body_player2_toolButton1_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[1][0]), ui->body_player2_text1);
}

void armor_edit::on_body_player2_toolButton2_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[1][1]), ui->body_player2_text2);
}

void armor_edit::on_body_player2_toolButton3_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[1][2]), ui->body_player2_text3);
}

void armor_edit::on_body_player3_toolButton1_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[2][0]), ui->body_player3_text1);
}

void armor_edit::on_body_player3_toolButton2_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[2][1]), ui->body_player3_text2);
}

void armor_edit::on_body_player3_toolButton3_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[2][2]), ui->body_player3_text3);
}

void armor_edit::on_body_player4_toolButton1_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[3][0]), ui->body_player4_text1);
}

void armor_edit::on_body_player4_toolButton2_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[3][1]), ui->body_player4_text2);
}

void armor_edit::on_body_player4_toolButton3_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_BODY].got_message[3][2]), ui->body_player4_text3);
}

void armor_edit::on_legs_player1_toolButton1_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[0][0]), ui->legs_player1_text1);
}

void armor_edit::on_legs_player1_toolButton2_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[0][1]), ui->legs_player1_text2);
}

void armor_edit::on_legs_player1_toolButton3_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[0][2]), ui->legs_player1_text3);
}

void armor_edit::on_legs_player2_toolButton1_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[1][0]), ui->legs_player2_text1);
}

void armor_edit::on_legs_player2_toolButton2_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[1][1]), ui->legs_player2_text2);
}

void armor_edit::on_legs_player2_toolButton3_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[1][2]), ui->legs_player2_text3);
}

void armor_edit::on_legs_player3_toolButton1_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[2][0]), ui->legs_player3_text1);
}

void armor_edit::on_legs_player3_toolButton2_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[2][1]), ui->legs_player3_text2);
}

void armor_edit::on_legs_player3_toolButton3_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[2][2]), ui->legs_player3_text3);
}

void armor_edit::on_legs_player4_toolButton1_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[3][0]), ui->legs_player4_text1);
}

void armor_edit::on_legs_player4_toolButton2_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[3][1]), ui->legs_player4_text2);
}

void armor_edit::on_legs_player4_toolButton3_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->game_data.armor_pieces[ARMOR_LEGS].got_message[3][2]), ui->legs_player4_text3);
}
