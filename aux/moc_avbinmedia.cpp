/****************************************************************************
** Meta object code from reading C++ file 'avbinmedia.h'
**
** Created: Sun Nov 25 13:49:48 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtMedia/avbinmedia.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'avbinmedia.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AvBinMedia[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      38,   35,   12,   11, 0x0a,
      81,   11,   62,   11, 0x0a,
      96,   11,   62,   11, 0x0a,
     105,   35,   62,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_AvBinMedia[] = {
    "AvBinMedia\0\0QSharedPointer<QImage>\0"
    "ti\0Get(long long unsigned)\0"
    "long long unsigned\0GetNumFrames()\0"
    "Length()\0GetFrameStartTime(long long unsigned)\0"
};

void AvBinMedia::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        AvBinMedia *_t = static_cast<AvBinMedia *>(_o);
        switch (_id) {
        case 0: { QSharedPointer<QImage> _r = _t->Get((*reinterpret_cast< long long unsigned(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QSharedPointer<QImage>*>(_a[0]) = _r; }  break;
        case 1: { long long unsigned _r = _t->GetNumFrames();
            if (_a[0]) *reinterpret_cast< long long unsigned*>(_a[0]) = _r; }  break;
        case 2: { long long unsigned _r = _t->Length();
            if (_a[0]) *reinterpret_cast< long long unsigned*>(_a[0]) = _r; }  break;
        case 3: { long long unsigned _r = _t->GetFrameStartTime((*reinterpret_cast< long long unsigned(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< long long unsigned*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData AvBinMedia::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject AvBinMedia::staticMetaObject = {
    { &AbstractMedia::staticMetaObject, qt_meta_stringdata_AvBinMedia,
      qt_meta_data_AvBinMedia, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AvBinMedia::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AvBinMedia::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AvBinMedia::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AvBinMedia))
        return static_cast<void*>(const_cast< AvBinMedia*>(this));
    return AbstractMedia::qt_metacast(_clname);
}

int AvBinMedia::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractMedia::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
