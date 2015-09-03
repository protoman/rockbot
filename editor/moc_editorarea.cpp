/****************************************************************************
** Meta object code from reading C++ file 'editorarea.h'
**
** Created: Sun Dec 6 09:18:11 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "editorarea.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'editorarea.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_EditorArea[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,
      25,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_EditorArea[] = {
    "EditorArea\0\0changeTile()\0saveGame()\0"
};

const QMetaObject EditorArea::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_EditorArea,
      qt_meta_data_EditorArea, 0 }
};

const QMetaObject *EditorArea::metaObject() const
{
    return &staticMetaObject;
}

void *EditorArea::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_EditorArea))
        return static_cast<void*>(const_cast< EditorArea*>(this));
    return QWidget::qt_metacast(_clname);
}

int EditorArea::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: changeTile(); break;
        case 1: saveGame(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
