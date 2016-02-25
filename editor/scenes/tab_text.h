#ifndef TEXT_H
#define TEXT_H

#include <QDialog>
#include <QLineEdit>
#include "scenes/scenesmediator.h"
#include "defines.h"
#include "file/fio_scenes.h"
#include "file/fio_strings.h"
#include "files_editor/stringseditor.h"

namespace Ui {
class TabText;
}

class TabText : public QDialog
{
    Q_OBJECT

public:
    explicit TabText(QWidget *parent = 0);
    ~TabText();
    void save_data();
    void reload();


private slots:
    void on_AddButton_clicked();

    void on_positionType_currentIndexChanged(int index);

    void on_select_comboBox_currentIndexChanged(int index);

    void on_line1_toolButton_clicked();

    void on_line2_toolButton_clicked();

    void on_line3_toolButton_clicked();

    void on_line4_toolButton_clicked();

    void on_line5_toolButton_clicked();

    void on_line6_toolButton_clicked();

    void on_name_lineEdit_textChanged(const QString &arg1);

    void on_removeString1_toolButton_clicked();

    void on_removeString2_toolButton_clicked();

    void on_removeString3_toolButton_clicked();

    void on_removeString4_toolButton_clicked();

    void on_removeString5_toolButton_clicked();

    void on_removeString6_toolButton_clicked();

    void on_textX_spinBox_valueChanged(int arg1);

    void on_textY_spinBox_valueChanged(int arg1);

public slots:
    void on_string_selected(int string_id);


private:
    void change_fields_enabled(bool state);
    void change_x_y_fields_enabled();
    void fill_data();
    void set_fields(int index);
    void string_tooltip_click(int *property, QLineEdit* qline);

private:
    Ui::TabText *ui;
    CURRENT_FILE_FORMAT::fio_scenes fio;
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    bool data_loading;
};

#endif // TEXT_H
