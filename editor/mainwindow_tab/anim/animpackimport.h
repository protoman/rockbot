#ifndef ANIMPACKIMPORT_H
#define ANIMPACKIMPORT_H

#include <QDialog>

namespace Ui {
class AnimPackImport;
}

class AnimPackImport : public QDialog
{
    Q_OBJECT

public:
    explicit AnimPackImport(QWidget *parent = 0);
    ~AnimPackImport();

private:
    Ui::AnimPackImport *ui;
};

#endif // ANIMPACKIMPORT_H
