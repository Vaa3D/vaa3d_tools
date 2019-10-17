/****************************************************************************
** Meta object code from reading C++ file 'mysocket.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mysocket.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mysocket.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MySocket[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x05,
      54,   50,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      98,    9,    9,    9, 0x0a,
     118,    9,    9,    9, 0x0a,
     144,   50,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MySocket[] = {
    "MySocket\0\0MySocketSignalToMyThread_disconnected()\0"
    "msg\0MySocketSignalToMyThread_sendtoall(QString)\0"
    "MySocketSlot_Read()\0MySocketSlot_disconnect()\0"
    "MySocketSlotAnswerMyThread_sendtouser(QString)\0"
};

void MySocket::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MySocket *_t = static_cast<MySocket *>(_o);
        switch (_id) {
        case 0: _t->MySocketSignalToMyThread_disconnected(); break;
        case 1: _t->MySocketSignalToMyThread_sendtoall((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->MySocketSlot_Read(); break;
        case 3: _t->MySocketSlot_disconnect(); break;
        case 4: _t->MySocketSlotAnswerMyThread_sendtouser((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MySocket::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MySocket::staticMetaObject = {
    { &QTcpSocket::staticMetaObject, qt_meta_stringdata_MySocket,
      qt_meta_data_MySocket, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MySocket::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MySocket::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MySocket::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MySocket))
        return static_cast<void*>(const_cast< MySocket*>(this));
    return QTcpSocket::qt_metacast(_clname);
}

int MySocket::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTcpSocket::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void MySocket::MySocketSignalToMyThread_disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void MySocket::MySocketSignalToMyThread_sendtoall(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
