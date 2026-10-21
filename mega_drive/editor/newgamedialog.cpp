#include "newgamedialog.h"
#include "ui_newgamedialog.h"

#include <QDir>

extern std::string GAMEPATH;

NewGameDialog::NewGameDialog(QWidget *parent) : QDialog(parent), ui(new Ui::NewGameDialog)
{
    ui->setupUi(this);
    data_loading = false;
    ui->buttonBox->setEnabled(false);
}

NewGameDialog::~NewGameDialog()
{
    delete ui;
}

bool NewGameDialog::game_folder_exists(QString name)
{
    QString filepath = QString(GAMEPATH.c_str()) + QString("/games/") + name;
    return QDir(filepath).exists();
}

void NewGameDialog::on_gameName_lineEdit_textChanged(const QString &arg1)
{
    if (data_loading) { return; }
    game_name = arg1;
    game_name.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}\'\"\\\[\\\]\\\\]")));
    if (game_name.size() < 5) {
        ui->error_label->setText("Name must consist of at least 5 alphanumeric only characters.");
        ui->buttonBox->setEnabled(false);
    } else if (game_folder_exists(game_name)) {
        ui->error_label->setText("A game with this name already exists.");
        ui->buttonBox->setEnabled(false);
    } else {
        ui->error_label->setText("");
        ui->buttonBox->setEnabled(true);
    }
    game_name = game_name.remove(" ");
    data_loading = true;
    ui->gameName_lineEdit->setText(game_name);
    data_loading = false;

}

void NewGameDialog::on_buttonBox_accepted()
{
    emit on_accepted(game_name);
}
