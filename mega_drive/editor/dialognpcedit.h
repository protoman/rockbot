#ifndef DIALOGNPCEDIT_H
#define DIALOGNPCEDIT_H

#include <QDialog>
#include <QAbstractButton>


namespace Ui {
    class DialogNPCEdit;
}

class DialogNPCEdit : public QDialog {
    Q_OBJECT
public:
    DialogNPCEdit(QWidget *parent = 0);
    ~DialogNPCEdit();
	void fillNPCGraphicsList();
	int editMode;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DialogNPCEdit *ui;
	void loadNPCData(int npc_n);
	void saveNPCData(int npc_n);

private slots:

void on_spinBox_valueChanged(int );
void on_buttonBox_accepted();
void on_frameTypeCombo_currentIndexChanged(int index);
void on_npcListCombobox_currentIndexChanged(QString );
void on_npcGraphicSizeSpin_h_valueChanged(int );
void on_npcGraphicSizeSpin_w_valueChanged(int );


public slots:
	void setNPCFrame();

signals:
	void finishedNPCEditor();

};

#endif // DIALOGNPCEDIT_H
