#ifndef ADDWIZARD_H
#define ADDWIZARD_H

#include <QWidget>
#include <QWizard>
#include "ui_addwizard.h"
#include "mediator.h"

class addWizard : public QWizard
 {
     Q_OBJECT

 public:
     addWizard(QWidget *parent = 0);

 private slots:
 void on_npcListCombobox_currentIndexChanged(QString );
	void on_npcGraphicSizeSpin_valueChanged(int );
	void on_comboBox_currentIndexChanged(int);
	void pageChanged(int id);
	int nextId() const;

 public:
	 int type;
	 int done;

 private:
     Ui::addWizard ui;
     int nextPageCount;
     int getCurrentPage();
     void fillNPCGraphicsList();

 signals:
	 void finishedWizard();


 };

#endif // ADDWIZARD_H
