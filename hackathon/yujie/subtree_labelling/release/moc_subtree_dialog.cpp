/****************************************************************************
** Meta object code from reading C++ file 'subtree_dialog.h'
**
** Created: Sat Feb 27 16:05:43 2016
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
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x0a,
      31,   15,   15,   15, 0x0a,
      46,   15,   15,   15, 0x0a,
      72,   15,   15,   15, 0x0a,
      88,   15,   15,   15, 0x0a,
     110,   15,   15,   15, 0x0a,
     127,   15,   15,   15, 0x0a,
     145,   15,   15,   15, 0x0a,
     167,   15,  162,   15, 0x0a,
     180,   15,  162,   15, 0x0a,
     187,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_subtree_dialog[] = {
    "subtree_dialog\0\0soma_clicked()\0"
    "axon_clicked()\0apical_dendrite_clicked()\0"
    "basal_clicked()\0apical_tuft_clicked()\0"
    "custom_clicked()\0oblique_clicked()\0"
    "refresh_marker()\0bool\0maybe_save()\0"
    "save()\0run()\0"
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
        case 0: soma_clicked(); break;
        case 1: axon_clicked(); break;
        case 2: apical_dendrite_clicked(); break;
        case 3: basal_clicked(); break;
        case 4: apical_tuft_clicked(); break;
        case 5: custom_clicked(); break;
        case 6: oblique_clicked(); break;
        case 7: refresh_marker(); break;
        case 8: { bool _r = maybe_save();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 9: { bool _r = save();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 10: run(); break;
        default: ;
        }
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
