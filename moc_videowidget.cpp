/****************************************************************************
** Meta object code from reading C++ file 'videowidget.h'
**
** Created: Mon Apr 8 19:26:45 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtMedia/videowidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'videowidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ZoomGraphicsView[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   18,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ZoomGraphicsView[] = {
    "ZoomGraphicsView\0\0event\0"
    "wheelEvent(QWheelEvent*)\0"
};

void ZoomGraphicsView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ZoomGraphicsView *_t = static_cast<ZoomGraphicsView *>(_o);
        switch (_id) {
        case 0: _t->wheelEvent((*reinterpret_cast< QWheelEvent*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ZoomGraphicsView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ZoomGraphicsView::staticMetaObject = {
    { &QGraphicsView::staticMetaObject, qt_meta_stringdata_ZoomGraphicsView,
      qt_meta_data_ZoomGraphicsView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ZoomGraphicsView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ZoomGraphicsView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ZoomGraphicsView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ZoomGraphicsView))
        return static_cast<void*>(const_cast< ZoomGraphicsView*>(this));
    return QGraphicsView::qt_metacast(_clname);
}

int ZoomGraphicsView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_VideoWidget[] = {

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
      34,   17,   13,   12, 0x0a,
      77,   68,   12,   12, 0x0a,
      94,   12,   12,   12, 0x0a,
     102,   12,   12,   12, 0x0a,
     109,   12,   12,   12, 0x0a,
     120,   12,   12,   12, 0x0a,
     134,   12,   12,   12, 0x0a,
     196,  148,   12,   12, 0x0a,
     296,  281,   12,   12, 0x0a,
     342,  334,   12,   12, 0x0a,
     368,   12,   12,   12, 0x0a,
     388,  382,   12,   12, 0x0a,
     413,  408,   12,   12, 0x0a,
     444,  432,   12,   12, 0x0a,
     472,  469,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_VideoWidget[] = {
    "VideoWidget\0\0int\0src,finaIn,error\0"
    "SetSource(QUuid,QString,QString&)\0"
    "newValue\0SliderMoved(int)\0Pause()\0"
    "Play()\0SeekBack()\0SeekForward()\0"
    "TimerUpdate()\0"
    "fr,startTimestamp,endTimestamp,requestTimestamp\0"
    "AsyncFrameReceived(QImage&,unsigned long long,unsigned long long,unsig"
    "ned long long)\0"
    "sceneControlIn\0SetSceneControl(BaseSceneController*)\0"
    "srcUuid\0SetAnnotationTrack(QUuid)\0"
    "FitToWindow()\0scale\0SetRawScale(double)\0"
    "time\0TimeChanged(QTime)\0eventLoopIn\0"
    "SetEventLoop(EventLoop*)\0ev\0"
    "HandleEvent(std::tr1::shared_ptr<Event>)\0"
};

void VideoWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        VideoWidget *_t = static_cast<VideoWidget *>(_o);
        switch (_id) {
        case 0: { int _r = _t->SetSource((*reinterpret_cast< QUuid(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 1: _t->SliderMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->Pause(); break;
        case 3: _t->Play(); break;
        case 4: _t->SeekBack(); break;
        case 5: _t->SeekForward(); break;
        case 6: _t->TimerUpdate(); break;
        case 7: _t->AsyncFrameReceived((*reinterpret_cast< QImage(*)>(_a[1])),(*reinterpret_cast< unsigned long long(*)>(_a[2])),(*reinterpret_cast< unsigned long long(*)>(_a[3])),(*reinterpret_cast< unsigned long long(*)>(_a[4]))); break;
        case 8: _t->SetSceneControl((*reinterpret_cast< BaseSceneController*(*)>(_a[1]))); break;
        case 9: _t->SetAnnotationTrack((*reinterpret_cast< QUuid(*)>(_a[1]))); break;
        case 10: _t->FitToWindow(); break;
        case 11: _t->SetRawScale((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 12: _t->TimeChanged((*reinterpret_cast< QTime(*)>(_a[1]))); break;
        case 13: _t->SetEventLoop((*reinterpret_cast< EventLoop*(*)>(_a[1]))); break;
        case 14: _t->HandleEvent((*reinterpret_cast< std::tr1::shared_ptr<Event>(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData VideoWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject VideoWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_VideoWidget,
      qt_meta_data_VideoWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &VideoWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *VideoWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *VideoWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_VideoWidget))
        return static_cast<void*>(const_cast< VideoWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int VideoWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
