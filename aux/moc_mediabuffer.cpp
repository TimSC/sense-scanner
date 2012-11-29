/****************************************************************************
** Meta object code from reading C++ file 'mediabuffer.h'
**
** Created: Sun Nov 25 13:41:00 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtMedia/mediabuffer.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mediabuffer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AbstractMedia[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      41,   38,   15,   14, 0x0a,
      78,   69,   65,   14, 0x0a,
     129,   14,  110,   14, 0x0a,
     144,   14,  110,   14, 0x0a,
     153,   38,  110,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_AbstractMedia[] = {
    "AbstractMedia\0\0QSharedPointer<QImage>\0"
    "ti\0Get(long long unsigned)\0int\0time,out\0"
    "GetFrame(int64_t,DecodedFrame&)\0"
    "long long unsigned\0GetNumFrames()\0"
    "Length()\0GetFrameStartTime(long long unsigned)\0"
};

void AbstractMedia::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        AbstractMedia *_t = static_cast<AbstractMedia *>(_o);
        switch (_id) {
        case 0: { QSharedPointer<QImage> _r = _t->Get((*reinterpret_cast< long long unsigned(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QSharedPointer<QImage>*>(_a[0]) = _r; }  break;
        case 1: { int _r = _t->GetFrame((*reinterpret_cast< int64_t(*)>(_a[1])),(*reinterpret_cast< DecodedFrame(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 2: { long long unsigned _r = _t->GetNumFrames();
            if (_a[0]) *reinterpret_cast< long long unsigned*>(_a[0]) = _r; }  break;
        case 3: { long long unsigned _r = _t->Length();
            if (_a[0]) *reinterpret_cast< long long unsigned*>(_a[0]) = _r; }  break;
        case 4: { long long unsigned _r = _t->GetFrameStartTime((*reinterpret_cast< long long unsigned(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< long long unsigned*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData AbstractMedia::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject AbstractMedia::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_AbstractMedia,
      qt_meta_data_AbstractMedia, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AbstractMedia::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AbstractMedia::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AbstractMedia::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AbstractMedia))
        return static_cast<void*>(const_cast< AbstractMedia*>(this));
    return QObject::qt_metacast(_clname);
}

int AbstractMedia::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_MediaBuffer[] = {

 // content:
       6,       // revision
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

static const char qt_meta_stringdata_MediaBuffer[] = {
    "MediaBuffer\0"
};

void MediaBuffer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MediaBuffer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MediaBuffer::staticMetaObject = {
    { &AbstractMedia::staticMetaObject, qt_meta_stringdata_MediaBuffer,
      qt_meta_data_MediaBuffer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MediaBuffer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MediaBuffer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MediaBuffer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MediaBuffer))
        return static_cast<void*>(const_cast< MediaBuffer*>(this));
    return AbstractMedia::qt_metacast(_clname);
}

int MediaBuffer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractMedia::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
