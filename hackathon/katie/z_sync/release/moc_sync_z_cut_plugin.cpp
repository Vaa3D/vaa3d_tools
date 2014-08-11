/****************************************************************************
** Meta object code from reading C++ file 'sync_z_cut_plugin.h'
**
** Created: Mon Aug 11 11:14:02 2014
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
static const uint qt_meta_data_MyComboBox[] = {

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
      12,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MyComboBox[] = {
    "MyComboBox\0\0updateList()\0"
};

const QMetaObject MyComboBox::staticMetaObject = {
    { &QComboBox::staticMetaObject, qt_meta_stringdata_MyComboBox,
      qt_meta_data_MyComboBox, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MyComboBox::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MyComboBox::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MyComboBox::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MyComboBox))
        return static_cast<void*>(const_cast< MyComboBox*>(this));
    return QComboBox::qt_metacast(_clname);
}

int MyComboBox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QComboBox::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updateList(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_lookPanel[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
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
      90,   10,   10,   10, 0x08,
     106,   10,   10,   10, 0x08,
     122,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_lookPanel[] = {
    "lookPanel\0\0reject()\0change_z_min()\0"
    "change_z_max()\0setZCutLockIcon(bool)\0"
    "setZCutLock(bool)\0update_traces()\0"
    "string_tester()\0update_sliders()\0"
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
        case 5: update_traces(); break;
        case 6: string_tester(); break;
        case 7: update_sliders(); break;
        default: ;
        }
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
