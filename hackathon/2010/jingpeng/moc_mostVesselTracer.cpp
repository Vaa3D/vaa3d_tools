/****************************************************************************
** Meta object code from reading C++ file 'mostVesselTracer.h'
**
** Created: Fri Sep 9 10:19:11 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mostVesselTracer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mostVesselTracer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_mostVesselTracerPlugin[] = {

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

static const char qt_meta_stringdata_mostVesselTracerPlugin[] = {
    "mostVesselTracerPlugin\0"
};

const QMetaObject mostVesselTracerPlugin::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_mostVesselTracerPlugin,
      qt_meta_data_mostVesselTracerPlugin, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &mostVesselTracerPlugin::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *mostVesselTracerPlugin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *mostVesselTracerPlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_mostVesselTracerPlugin))
        return static_cast<void*>(const_cast< mostVesselTracerPlugin*>(this));
    if (!strcmp(_clname, "V3DPluginInterface2_1"))
        return static_cast< V3DPluginInterface2_1*>(const_cast< mostVesselTracerPlugin*>(this));
    if (!strcmp(_clname, "com.janelia.v3d.V3DPluginInterface/2.1"))
        return static_cast< V3DPluginInterface2_1*>(const_cast< mostVesselTracerPlugin*>(this));
    return QObject::qt_metacast(_clname);
}

int mostVesselTracerPlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
