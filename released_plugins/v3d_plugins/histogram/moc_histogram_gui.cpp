/****************************************************************************
** Meta object code from reading C++ file 'histogram_gui.h'
**
** Created: Thu Mar 1 16:36:25 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "histogram_gui.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'histogram_gui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_HistogramDialog[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x0a,
      26,   16,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_HistogramDialog[] = {
    "HistogramDialog\0\0accept()\0reject()\0"
};

const QMetaObject HistogramDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_HistogramDialog,
      qt_meta_data_HistogramDialog, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &HistogramDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *HistogramDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *HistogramDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HistogramDialog))
        return static_cast<void*>(const_cast< HistogramDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int HistogramDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: accept(); break;
        case 1: reject(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
