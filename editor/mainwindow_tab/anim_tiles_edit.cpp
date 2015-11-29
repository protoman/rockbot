#include "anim_tiles_edit.h"
#include "ui_anim_tiles_edit.h"
#include "mediator.h"
#include "common.h"

anim_tiles_edit::anim_tiles_edit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::anim_tiles_edit)
{
    ui->setupUi(this);
    QObject::connect(ui->animTileFrames_widget, SIGNAL(clickedIn()), this, SLOT(onChangeFrame()));
    Mediator::get_instance()->selectedAnimTileset = 0;
    reload();
}

anim_tiles_edit::~anim_tiles_edit()
{
    delete ui;
}

void anim_tiles_edit::reload()
{
    data_loading = true;
    fill_data();
    data_loading = false;
}

void anim_tiles_edit::fill_data()
{
    common::fill_anim_block_combo(ui->currentBlock_combo);
    common::fill_files_combo("/images/tilesets/anim", ui->graphic_combo);
    set_data(0);
}

void anim_tiles_edit::set_data(int index)
{
    int max = Mediator::get_instance()->anim_block_list.size();
    if (max < 1 || index >= max) {
        ui->currentBlock_combo->setCurrentIndex(-1);
        ui->graphic_combo->setCurrentIndex(-1);
        return;
    }
    ui->graphic_combo->setCurrentIndex(ui->graphic_combo->findText(QString(Mediator::get_instance()->anim_block_list.at(index).filename)));
    std::string filename = FILEPATH + std::string("images/tilesets/anim/") + std::string(Mediator::get_instance()->anim_block_list.at(index).filename);
    std::cout << "FILENAME: " << filename << std::endl;
    ui->animTileFrames_widget->set_graphicfile(filename);
    ui->animTileFrames_widget->set_sprite_pos(st_position(0, 0));
    ui->animTileFrames_widget->repaint();
    ui->frameDelay_spinBox->setValue(Mediator::get_instance()->anim_block_list.at(index).frame_delay[0]);
    ui->animTilePreview_widget->update_properties();
}

void anim_tiles_edit::onChangeFrame()
{
    if (data_loading == true) { return; }
    data_loading = true;
    ui->frameDelay_spinBox->setValue(Mediator::get_instance()->anim_block_list.at(ui->currentBlock_combo->currentIndex()).frame_delay[Mediator::get_instance()->getPalleteX()]);
    data_loading = false;
}

void anim_tiles_edit::on_addBlock_button_clicked()
{
    Mediator::get_instance()->anim_block_list.push_back(CURRENT_FILE_FORMAT::file_anim_block());
    data_loading = true;
    common::fill_anim_block_combo(ui->currentBlock_combo);
    ui->currentBlock_combo->setCurrentIndex(Mediator::get_instance()->anim_block_list.size()-1);
    Mediator::get_instance()->selectedAnimTileset = Mediator::get_instance()->anim_block_list.size()-1;
    set_data(Mediator::get_instance()->anim_block_list.size()-1);
    data_loading = false;
}

void anim_tiles_edit::on_graphic_combo_currentIndexChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    std::string filename = FILEPATH + std::string("images/tilesets/anim/") + arg1.toStdString();
    ui->animTileFrames_widget->set_graphicfile(filename);
    ui->animTileFrames_widget->set_sprite_pos(st_position(0, 0));
    ui->animTileFrames_widget->repaint();
    sprintf(Mediator::get_instance()->anim_block_list.at(ui->currentBlock_combo->currentIndex()).filename, "%s", arg1.toStdString().c_str());
    ui->animTilePreview_widget->update_properties();
}

void anim_tiles_edit::on_frameDelay_spinBox_valueChanged(int arg1)
{
    if (data_loading == true) { return; }
    int posx = Mediator::get_instance()->getPalleteX();
    Mediator::get_instance()->anim_block_list.at(ui->currentBlock_combo->currentIndex()).frame_delay[posx] = arg1;
    ui->animTilePreview_widget->update_properties();
}

void anim_tiles_edit::on_currentBlock_combo_currentIndexChanged(int index)
{
    if (data_loading == true) { return; }
    data_loading = true;
    set_data(index);
    Mediator::get_instance()->selectedAnimTileset = index;
    ui->animTilePreview_widget->update_properties();
    data_loading = false;
}

void anim_tiles_edit::on_pushButton_clicked()
{
    AnimPackImport *import_window = new AnimPackImport();
    import_window->show();
}
