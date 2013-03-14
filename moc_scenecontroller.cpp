/****************************************************************************
** Meta object code from reading C++ file 'scenecontroller.h'
**
** Created: Tue Mar 12 18:59:52 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtMedia/scenecontroller.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'scenecontroller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BaseSceneController[] = {

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

static const char qt_meta_stringdata_BaseSceneController[] = {
    "BaseSceneController\0"
};

void BaseSceneController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData BaseSceneController::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject BaseSceneController::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_BaseSceneController,
      qt_meta_data_BaseSceneController, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BaseSceneController::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BaseSceneController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BaseSceneController::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BaseSceneController))
        return static_cast<void*>(const_cast< BaseSceneController*>(this));
    return QObject::qt_metacast(_clname);
}

int BaseSceneController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_TrackingSceneController[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      29,   25,   24,   24, 0x0a,
      52,   24,   24,   24, 0x0a,
      66,   24,   24,   24, 0x0a,
      83,   24,   24,   24, 0x0a,
     101,   24,   24,   24, 0x0a,
     122,   24,   24,   24, 0x0a,
     139,   24,   24,   24, 0x0a,
     159,   24,   24,   24, 0x0a,
     176,  171,   24,   24, 0x0a,
     195,   24,   24,   24, 0x0a,
     207,   24,   24,   24, 0x0a,
     233,   24,   24,   24, 0x0a,
     257,   24,   24,   24, 0x0a,
     274,   24,   24,   24, 0x0a,
     299,  291,   24,   24, 0x0a,
     337,  325,   24,   24, 0x0a,
     362,   24,   24,   24, 0x0a,
     382,   24,   24,   24, 0x0a,
     397,   24,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_TrackingSceneController[] = {
    "TrackingSceneController\0\0val\0"
    "MarkFramePressed(bool)\0MovePressed()\0"
    "MoveAllPressed()\0AddPointPressed()\0"
    "RemovePointPressed()\0AddLinkPressed()\0"
    "RemoveLinkPressed()\0LoadShape()\0fina\0"
    "LoadShape(QString)\0SaveShape()\0"
    "SetShapeFromCurentFrame()\0"
    "ResetCurentFrameShape()\0LoadAnnotation()\0"
    "SaveAnnotation()\0srcUuid\0"
    "SetAnnotationTrack(QUuid)\0eventLoopIn\0"
    "SetEventLoop(EventLoop*)\0RefreshCurrentPos()\0"
    "RefreshLinks()\0Update()\0"
};

void TrackingSceneController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        TrackingSceneController *_t = static_cast<TrackingSceneController *>(_o);
        switch (_id) {
        case 0: _t->MarkFramePressed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->MovePressed(); break;
        case 2: _t->MoveAllPressed(); break;
        case 3: _t->AddPointPressed(); break;
        case 4: _t->RemovePointPressed(); break;
        case 5: _t->AddLinkPressed(); break;
        case 6: _t->RemoveLinkPressed(); break;
        case 7: _t->LoadShape(); break;
        case 8: _t->LoadShape((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 9: _t->SaveShape(); break;
        case 10: _t->SetShapeFromCurentFrame(); break;
        case 11: _t->ResetCurentFrameShape(); break;
        case 12: _t->LoadAnnotation(); break;
        case 13: _t->SaveAnnotation(); break;
        case 14: _t->SetAnnotationTrack((*reinterpret_cast< QUuid(*)>(_a[1]))); break;
        case 15: _t->SetEventLoop((*reinterpret_cast< EventLoop*(*)>(_a[1]))); break;
        case 16: _t->RefreshCurrentPos(); break;
        case 17: _t->RefreshLinks(); break;
        case 18: _t->Update(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData TrackingSceneController::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject TrackingSceneController::staticMetaObject = {
    { &BaseSceneController::staticMetaObject, qt_meta_stringdata_TrackingSceneController,
      qt_meta_data_TrackingSceneController, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TrackingSceneController::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TrackingSceneController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TrackingSceneController::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TrackingSceneController))
        return static_cast<void*>(const_cast< TrackingSceneController*>(this));
    return BaseSceneController::qt_metacast(_clname);
}

int TrackingSceneController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BaseSceneController::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    return _id;
}
static const uint qt_meta_data_LogoSceneController[] = {

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

static const char qt_meta_stringdata_LogoSceneController[] = {
    "LogoSceneController\0"
};

void LogoSceneController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData LogoSceneController::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject LogoSceneController::staticMetaObject = {
    { &BaseSceneController::staticMetaObject, qt_meta_stringdata_LogoSceneController,
      qt_meta_data_LogoSceneController, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &LogoSceneController::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *LogoSceneController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *LogoSceneController::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_LogoSceneController))
        return static_cast<void*>(const_cast< LogoSceneController*>(this));
    return BaseSceneController::qt_metacast(_clname);
}

int LogoSceneController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BaseSceneController::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
