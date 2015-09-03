#ifndef DIALOGOBJECTEDIT_H
#define DIALOGOBJECTEDIT_H

#include <QDialog>

namespace Ui {
    class DialogObjectEdit;
}

class DialogObjectEdit : public QDialog
{
    Q_OBJECT

public:
    explicit DialogObjectEdit(QWidget *parent = 0);
    ~DialogObjectEdit();
	void fillObjectGraphicsList();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DialogObjectEdit *ui;
	void loadObjectData(int object_n);
	void saveObjectData(int object_n);


public slots:
	void setObjectFrame();

signals:
	void finishedObjectEditor();


private slots:
	void on_buttonBox_rejected();
	void on_objectListCombobox_currentIndexChanged(QString );
	void on_npcGraphicSizeSpin_h_valueChanged(int );
	void on_npcGraphicSizeSpin_w_valueChanged(int );
	void on_buttonBox_accepted();
	void on_NpcName_textChanged(const QString &arg1);
};



#endif // DIALOGOBJECTEDIT_H
