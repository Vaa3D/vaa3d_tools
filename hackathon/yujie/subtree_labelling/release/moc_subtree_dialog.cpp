/****************************************************************************
** Meta object code from reading C++ file 'subtree_dialog.h'
**
** Created: Sat Jun 20 23:33:09 2015
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../subtree_dialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'subtree_dialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_subtree_dialog[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_subtree_dialog[] = {
    "subtree_dialog\0\0marker_change()\0"
};

const QMetaObject subtree_dialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_subtree_dialog,
      qt_meta_data_subtree_dialog, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &subtree_dialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *subtree_dialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *subtree_dialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_subtree_dialog))
        return static_cast<void*>(const_cast< subtree_dialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int subtree_dialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: marker_change(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
