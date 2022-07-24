/****************************************************************************
** Meta object code from reading C++ file 'im.h'
**
** Created: Sat Sep 22 06:19:46 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "im.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'im.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_IMFrame[] = {

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
       9,    8,    8,    8, 0x08,
      30,    8,    8,    8, 0x08,
      41,    8,    8,    8, 0x08,
      53,    8,    8,    8, 0x08,
      70,   65,    8,    8, 0x08,
      87,    8,    8,    8, 0x08,
      98,    8,    8,    8, 0x08,
     123,  110,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_IMFrame[] = {
    "IMFrame\0\0sendContent(QString)\0setShift()\0"
    "setChaNum()\0setEngChn()\0text\0"
    "transPy(QString)\0turnLeft()\0turnRight()\0"
    "message,data\0handleMessageFromQCop(QString,QByteArray)\0"
};

const QMetaObject IMFrame::staticMetaObject = {
    { &QWSInputMethod::staticMetaObject, qt_meta_stringdata_IMFrame,
      qt_meta_data_IMFrame, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &IMFrame::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *IMFrame::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *IMFrame::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_IMFrame))
        return static_cast<void*>(const_cast< IMFrame*>(this));
    return QWSInputMethod::qt_metacast(_clname);
}

int IMFrame::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWSInputMethod::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: sendContent((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: setShift(); break;
        case 2: setChaNum(); break;
        case 3: setEngChn(); break;
        case 4: transPy((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: turnLeft(); break;
        case 6: turnRight(); break;
        case 7: handleMessageFromQCop((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
