/****************************************************************************
** Meta object code from reading C++ file 'sync_z_cut_plugin.h'
**
** Created: Mon Jul 14 15:01:23 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../sync_z_cut_plugin.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sync_z_cut_plugin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SyncZ[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_SyncZ[] = {
    "SyncZ\0"
};

const QMetaObject SyncZ::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SyncZ,
      qt_meta_data_SyncZ, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SyncZ::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SyncZ::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SyncZ::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SyncZ))
        return static_cast<void*>(const_cast< SyncZ*>(this));
    if (!strcmp(_clname, "V3DPluginInterface2_1"))
        return static_cast< V3DPluginInterface2_1*>(const_cast< SyncZ*>(this));
    if (!strcmp(_clname, "com.janelia.v3d.V3DPluginInterface/2.1"))
        return static_cast< V3DPluginInterface2_1*>(const_cast< SyncZ*>(this));
    return QObject::qt_metacast(_clname);
}

int SyncZ::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_lookPanel[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x08,
      20,   10,   10,   10, 0x08,
      35,   10,   10,   10, 0x08,
      50,   10,   10,   10, 0x08,
      72,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_lookPanel[] = {
    "lookPanel\0\0reject()\0change_z_min()\0"
    "change_z_max()\0setZCutLockIcon(bool)\0"
    "setZCutLock(bool)\0"
};

const QMetaObject lookPanel::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_lookPanel,
      qt_meta_data_lookPanel, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &lookPanel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *lookPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *lookPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_lookPanel))
        return static_cast<void*>(const_cast< lookPanel*>(this));
    return QDialog::qt_metacast(_clname);
}

int lookPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: reject(); break;
        case 1: change_z_min(); break;
        case 2: change_z_max(); break;
        case 3: setZCutLockIcon((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: setZCutLock((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
