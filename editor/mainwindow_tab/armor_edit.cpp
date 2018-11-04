#include "armor_edit.h"
#include "common.h"

armor_edit::armor_edit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::armor_edit)
{
    ui->setupUi(this);
    _data_loading = true;
    common::fill_players_combo(ui->playerSelect_comboBox);
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
    if (ui->playerSelect_comboBox->currentIndex() < 0) {
        return;
    }

    _data_loading = true;
    common::fill_projectiles_combo(ui->arm_comboBox, true);

    ui->legs_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_TYPE_LEGS].special_ability[ui->playerSelect_comboBox->currentIndex()]);
    ui->body_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_TYPE_BODY].special_ability[ui->playerSelect_comboBox->currentIndex()]);

    int arms_n = Mediator::get_instance()->game_data.armor_pieces[ARMOR_TYPE_ARMS].special_ability[ui->playerSelect_comboBox->currentIndex()];
    //std::cout << "ARMS[" << arms_n << "]" << std::endl;

    ui->arm_comboBox->setCurrentIndex(Mediator::get_instance()->game_data.armor_pieces[ARMOR_TYPE_ARMS].special_ability[ui->playerSelect_comboBox->currentIndex()]+1);

    _data_loading = false;

}

void armor_edit::fill_armor_abilities()
{
    std::string legs_abilities[] = {"Double Jump", "Air-Dash", "Shoryuken"};
    std::string body_abilities[] = {"Half-Damage", "Extended Immunity", "Spikes Immune", "No Push-Back"};


    for (unsigned int i = 0; i<4; i++) {
        ui->body_comboBox->addItem(body_abilities[i].c_str());
    }

    for (unsigned int i = 0; i<3; i++) {
        ui->legs_comboBox->addItem(legs_abilities[i].c_str());
    }

    reload();
}


void armor_edit::on_string_selected(int string_id)
{
    StringsEditor* strings_editor_window = (StringsEditor*)sender();
    int* value_property = strings_editor_window->get_target_property();
    *value_property = string_id;
    QLineEdit* qline = strings_editor_window->get_target_qline();
}


void armor_edit::string_tooltip_click(int *property, QLineEdit *qline)
{
    StringsEditor* strings_editor_window = new StringsEditor(this, 1);
    QObject::connect(strings_editor_window, SIGNAL(on_accepted(int)), this, SLOT(on_string_selected(int)));
    strings_editor_window->set_target_property(property);
    strings_editor_window->set_target_qline(qline);
    strings_editor_window->show();
}



void armor_edit::on_arm_comboBox_currentIndexChanged(int index)
{
    if (_data_loading) { return; }
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_TYPE_ARMS].special_ability[ui->playerSelect_comboBox->currentIndex()] = index-1;
}

void armor_edit::on_body_comboBox_currentIndexChanged(int index)
{
    if (_data_loading) { return; }
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_TYPE_BODY].special_ability[ui->playerSelect_comboBox->currentIndex()] = index;
}

void armor_edit::on_legs_comboBox_currentIndexChanged(int index)
{
    if (_data_loading) { return; }
    Mediator::get_instance()->game_data.armor_pieces[ARMOR_TYPE_LEGS].special_ability[ui->playerSelect_comboBox->currentIndex()] = index;
}

void armor_edit::on_playerSelect_comboBox_currentIndexChanged(int index)
{
    if (_data_loading) { return; }
    reload();
}
