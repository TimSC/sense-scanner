/****************************************************************************
** Meta object code from reading C++ file 'scenecontroller.h'
**
** Created: Sat Jan 12 16:36:36 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtMedia/scenecontroller.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'scenecontroller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TrackingAnnotation[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   20,   19,   19, 0x0a,
      47,   19,   19,   19, 0x0a,
      61,   19,   19,   19, 0x0a,
      78,   19,   19,   19, 0x0a,
      96,   19,   19,   19, 0x0a,
     117,   19,   19,   19, 0x0a,
     134,   19,   19,   19, 0x0a,
     154,   19,   19,   19, 0x0a,
     166,   19,   19,   19, 0x0a,
     178,   19,   19,   19, 0x0a,
     204,   19,   19,   19, 0x0a,
     228,   19,   19,   19, 0x0a,
     245,   19,   19,   19, 0x0a,
     267,  262,   19,   19, 0x0a,
     303,  299,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_TrackingAnnotation[] = {
    "TrackingAnnotation\0\0val\0MarkFramePressed(bool)\0"
    "MovePressed()\0MoveAllPressed()\0"
    "AddPointPressed()\0RemovePointPressed()\0"
    "AddLinkPressed()\0RemoveLinkPressed()\0"
    "LoadShape()\0SaveShape()\0"
    "SetShapeFromCurentFrame()\0"
    "ResetCurentFrameShape()\0LoadAnnotation()\0"
    "SaveAnnotation()\0elem\0"
    "ReadAnnotationXml(QDomElement&)\0out\0"
    "WriteAnnotationXml(QTextStream&)\0"
};

void TrackingAnnotation::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        TrackingAnnotation *_t = static_cast<TrackingAnnotation *>(_o);
        switch (_id) {
        case 0: _t->MarkFramePressed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->MovePressed(); break;
        case 2: _t->MoveAllPressed(); break;
        case 3: _t->AddPointPressed(); break;
        case 4: _t->RemovePointPressed(); break;
        case 5: _t->AddLinkPressed(); break;
        case 6: _t->RemoveLinkPressed(); break;
        case 7: _t->LoadShape(); break;
        case 8: _t->SaveShape(); break;
        case 9: _t->SetShapeFromCurentFrame(); break;
        case 10: _t->ResetCurentFrameShape(); break;
        case 11: _t->LoadAnnotation(); break;
        case 12: _t->SaveAnnotation(); break;
        case 13: _t->ReadAnnotationXml((*reinterpret_cast< QDomElement(*)>(_a[1]))); break;
        case 14: _t->WriteAnnotationXml((*reinterpret_cast< QTextStream(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData TrackingAnnotation::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject TrackingAnnotation::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_TrackingAnnotation,
      qt_meta_data_TrackingAnnotation, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TrackingAnnotation::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TrackingAnnotation::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TrackingAnnotation::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TrackingAnnotation))
        return static_cast<void*>(const_cast< TrackingAnnotation*>(this));
    return QObject::qt_metacast(_clname);
}

int TrackingAnnotation::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
