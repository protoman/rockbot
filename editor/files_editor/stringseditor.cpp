#include "stringseditor.h"
#include "ui_stringseditor.h"
#include "aux_tools/stringutils.h"

#include <QMessageBox>
#include <QCloseEvent>


extern std::string FILEPATH;

StringsEditor::StringsEditor(QWidget *parent, int mode) : QDialog(parent), ui(new Ui::StringsEditor), string_edit_model(this), scenes_string_edit_model(this), target_qline(NULL), target_property(NULL)
{
    pick_mode = mode;
    string_list = fio_str.load_game_strings();
    ui->setupUi(this);

    data_loading = true;


    // ==================== GAME STRINGS ==================== //
    fill_data();

    QObject::connect(&signal_mapper, SIGNAL(mapped(const int &)), this, SLOT(on_text_changed(const int &)));

    for (int i=0; i<string_list.size(); i++) {
        QLabel* line_text = new QLabel();
        line_text->setText(QString(string_list.at(i).c_str()));
        ui->tabGame_gridLayout->addWidget(line_text, i, 0);

        QLineEdit* line_editor = new QLineEdit();
        editor_line_list.push_back(line_editor);
        line_editor->setEnabled(false);
        ui->tabGame_gridLayout->addWidget(line_editor, i, 1);
        signal_mapper.setMapping(line_editor, i);
        QObject::connect(line_editor, SIGNAL(textChanged(QString)), &signal_mapper, SLOT(map()));
    }

    if (ui->languageSelector_comboBox->count() > 0) {
        current_lang = ui->languageSelector_comboBox->currentText().toStdString();
        std::string path_new = FILEPATH + std::string("/lang/strings_ingame_") + current_lang + std::string(".dat");
        load_language(path_new);
    }

    // ==================== COMMON STRINGS ==================== //

    std::vector<std::string> common_string_list = fio_str.get_common_strings();
    string_edit_model.set_data(common_string_list);
    string_edit_model.set_pick_mode(pick_mode);
    ui->commonStrings_tableView->setModel(&string_edit_model);
#if QT_VERSION >= 0x050000
    ui->commonStrings_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // Qt5
#else
    ui->commonStrings_tableView->horizontalHeader()->setStretchLastSection(true); // Qt4
#endif



    // ==================== SCENES STRINGS ==================== //

    std::vector<std::string> scenes_string_list = fio_str.get_scenes_strings();
    scenes_string_edit_model.set_data(scenes_string_list);
    scenes_string_edit_model.set_pick_mode(pick_mode);
    ui->scenesStrings_tableView->setModel(&scenes_string_edit_model);
#if QT_VERSION >= 0x050000
    ui->scenesStrings_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // Qt5
#else
    ui->scenesStrings_tableView->horizontalHeader()->setStretchLastSection(true); // Qt4
#endif

    data_loading = false;

    if (pick_mode != pick_mode_edit) {
        ui->tabWidget->setTabEnabled(0, false);
        ui->addLanguage_pushButton->setVisible(false);
        ui->languageName_lineEdit->setVisible(false);
        ui->addCommonString_pushButton->setEnabled(false);
        ui->addCommonString_pushButton->setVisible(false);
        ui->addScenesString_pushButton->setEnabled(false);
        ui->addScenesString_pushButton->setVisible(false);
        if (pick_mode == pick_mode_common) {
            ui->tabWidget->setCurrentIndex(1);
            ui->tabWidget->setTabEnabled(2, false);
        } else if (pick_mode == pick_mode_scenes) {
            ui->tabWidget->setCurrentIndex(2);
            ui->tabWidget->setTabEnabled(1, false);
        }

    }
}


void StringsEditor::save()
{
    fio_str.save_common_strings(string_edit_model.get_data());
    fio_str.save_scenes_strings(scenes_string_edit_model.get_data());
    ui->gameCredits_widget->save_data();
}

StringsEditor::~StringsEditor()
{
    delete ui;
}

void StringsEditor::save_data()
{
    if (current_lang.length() != 2) {
        return;
    }
    fio_str.save_game_strings(translation_string_list, fio_str.get_game_strings_filename());
}

void StringsEditor::set_target_qline(QLineEdit *line)
{
    target_qline = line;
}

QLineEdit *StringsEditor::get_target_qline()
{
    return target_qline;
}

void StringsEditor::set_target_property(int *prop)
{
    target_property = prop;
}

int *StringsEditor::get_target_property()
{
    return target_property;
}

int StringsEditor::get_pick_mode()
{
    return pick_mode;
}

void StringsEditor::closeEvent(QCloseEvent *event)
{
    if (pick_mode == pick_mode_edit) {
        QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Rockbot Editor :: Movie Editor", tr("Save data before leaving?\n"), QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
        if (resBtn == QMessageBox::Yes) {
            save();
        }
    }
    event->accept();
}

void StringsEditor::fill_data()
{
    std::string lang_dir_filename = FILEPATH + std::string("/lang/");
    lang_dir_filename = StringUtils::clean_filename(lang_dir_filename);
    if (QDir(lang_dir_filename.c_str()).exists() == false) {
        QDir().mkdir(lang_dir_filename.c_str());
        return;
    }
    std::vector<std::string> lang_list = fio.read_file_list(lang_dir_filename);
    for (int i=0; i<lang_list.size(); i++) {
        std::string lang_name = lang_list.at(i);
        StringUtils::replace_all(lang_name, "strings_ingame_", "");
        StringUtils::replace_all(lang_name, ".dat", "");
        ui->languageSelector_comboBox->addItem(lang_name.c_str());
    }
    if (lang_list.size() > 0) {
        ui->languageSelector_comboBox->setEnabled(true);
    }
}

void StringsEditor::fill_translation()
{
    for (int i=0; i<translation_string_list.size(); i++) {
        QLineEdit* line = editor_line_list.at(i);
        line->setText(translation_string_list.at(i).c_str());
        line->setEnabled(true);
    }
}



void StringsEditor::on_text_changed(int n)
{
    if (data_loading) { return; }
    QLineEdit* line = editor_line_list.at(n);
    std::string line_value = line->text().toStdString();
    translation_string_list.at(n) = line_value;
}

void StringsEditor::on_buttonBox_accepted()
{
    QModelIndexList selectedList;
    if (pick_mode == pick_mode_common) {
        selectedList = ui->commonStrings_tableView->selectionModel()->selectedRows();
    } else if (pick_mode == pick_mode_scenes) {
        selectedList = ui->scenesStrings_tableView->selectionModel()->selectedRows();
    }
    for (int i=0; i<selectedList.count(); i++) {
        emit on_accepted(selectedList.at(i).row());
        break;
    }
    if (pick_mode == pick_mode_edit) {
        save_data();
    }
    this->close();
}

void StringsEditor::on_buttonBox_rejected()
{
    this->close();
}

void StringsEditor::load_language(std::string filename)
{
    translation_string_list = fio_str.load_game_strings_from_file(filename);
    fill_translation();

}


void StringsEditor::on_addLanguage_pushButton_clicked()
{
    data_loading = true;
    std::string path_old = FILEPATH + std::string("/strings_ingame_v4.dat");
    std::string path_new = FILEPATH + std::string("/lang/strings_ingame_") + ui->languageName_lineEdit->text().toStdString() + std::string(".dat");
    QFile::copy(path_old.c_str(), path_new.c_str());
    load_language(path_new);
    ui->languageSelector_comboBox->addItem(ui->languageName_lineEdit->text().toStdString().c_str());
    ui->languageSelector_comboBox->setEnabled(true);
    data_loading = false;
}

void StringsEditor::on_languageName_lineEdit_textChanged(const QString &arg1)
{
    std::cout << ">>> #1 StringsEditor::on_languageName_lineEdit_textChanged, len: " << arg1.length() << std::endl;
    if (data_loading) { return; }
    std::cout << ">>> #2 StringsEditor::on_languageName_lineEdit_textChanged, len: " << arg1.length() << std::endl;
    if (arg1.length() == 2) {
        ui->addLanguage_pushButton->setEnabled(true);
    } else {
        ui->addLanguage_pushButton->setEnabled(false);
    }
}

void StringsEditor::on_languageSelector_comboBox_currentTextChanged(const QString &arg1)
{
    if (data_loading) { return; }
    save_data();
    current_lang = arg1.toStdString();
    data_loading = true;
    std::string path_new = FILEPATH + std::string("/lang/strings_ingame_") + arg1.toStdString() + std::string(".dat");
    load_language(path_new);
    fill_translation();
    data_loading = false;
}

void StringsEditor::on_addCommonString_pushButton_clicked()
{
    string_edit_model.add_line();
}

void StringsEditor::on_addScenesString_pushButton_clicked()
{
    scenes_string_edit_model.add_line();
}
