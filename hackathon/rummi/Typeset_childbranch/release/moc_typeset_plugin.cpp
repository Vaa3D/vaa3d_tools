/****************************************************************************
** Meta object code from reading C++ file 'typeset_plugin.h'
**
** Created: Thu Jul 31 11:12:57 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../typeset_plugin.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'typeset_plugin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TypesetPlugin[] = {

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

static const char qt_meta_stringdata_TypesetPlugin[] = {
    "TypesetPlugin\0"
};

const QMetaObject TypesetPlugin::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_TypesetPlugin,
      qt_meta_data_TypesetPlugin, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TypesetPlugin::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TypesetPlugin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TypesetPlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TypesetPlugin))
        return static_cast<void*>(const_cast< TypesetPlugin*>(this));
    if (!strcmp(_clname, "V3DPluginInterface2_1"))
        return static_cast< V3DPluginInterface2_1*>(const_cast< TypesetPlugin*>(this));
    if (!strcmp(_clname, "com.janelia.v3d.V3DPluginInterface/2.1"))
        return static_cast< V3DPluginInterface2_1*>(const_cast< TypesetPlugin*>(this));
    return QObject::qt_metacast(_clname);
}

int TypesetPlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
static const uint qt_meta_data_controlPanel[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x08,
      27,   13,   13,   13, 0x08,
      47,   13,   13,   13, 0x08,
      70,   13,   13,   13, 0x08,
      86,   13,   13,   13, 0x08,
      99,   13,   13,   13, 0x08,
     119,   13,  112,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_controlPanel[] = {
    "controlPanel\0\0_slot_sort()\0"
    "_slot_typeset_all()\0_slot_typeset_marker()\0"
    "_slot_refresh()\0_slot_show()\0_slot_save()\0"
    "double\0get_type()\0"
};

const QMetaObject controlPanel::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_controlPanel,
      qt_meta_data_controlPanel, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &controlPanel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *controlPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *controlPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_controlPanel))
        return static_cast<void*>(const_cast< controlPanel*>(this));
    return QDialog::qt_metacast(_clname);
}

int controlPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _slot_sort(); break;
        case 1: _slot_typeset_all(); break;
        case 2: _slot_typeset_marker(); break;
        case 3: _slot_refresh(); break;
        case 4: _slot_show(); break;
        case 5: _slot_save(); break;
        case 6: { double _r = get_type();
            if (_a[0]) *reinterpret_cast< double*>(_a[0]) = _r; }  break;
        default: ;
        }
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
