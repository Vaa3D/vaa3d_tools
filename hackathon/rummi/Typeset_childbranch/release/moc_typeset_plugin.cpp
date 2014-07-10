/****************************************************************************
** Meta object code from reading C++ file 'typeset_plugin.h'
**
** Created: Thu Jul 3 12:09:40 2014
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
QT_END_MOC_NAMESPACE
