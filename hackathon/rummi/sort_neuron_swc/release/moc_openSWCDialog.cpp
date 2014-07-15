/****************************************************************************
** Meta object code from reading C++ file 'openSWCDialog.h'
**
** Created: Mon Jul 14 12:15:26 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../openSWCDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'openSWCDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_OpenSWCDialog[] = {

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
      20,   14,   15,   14, 0x0a,
      31,   26,   15,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_OpenSWCDialog[] = {
    "OpenSWCDialog\0\0bool\0run()\0file\0"
    "setTree(QString)\0"
};

const QMetaObject OpenSWCDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_OpenSWCDialog,
      qt_meta_data_OpenSWCDialog, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &OpenSWCDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *OpenSWCDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *OpenSWCDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OpenSWCDialog))
        return static_cast<void*>(const_cast< OpenSWCDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int OpenSWCDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: { bool _r = run();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 1: { bool _r = setTree((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
