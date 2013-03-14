/****************************************************************************
** Meta object code from reading C++ file 'aboutgui.h'
**
** Created: Thu Mar 14 11:05:00 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtMedia/aboutgui.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'aboutgui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AboutGui[] = {

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

static const char qt_meta_stringdata_AboutGui[] = {
    "AboutGui\0"
};

void AboutGui::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData AboutGui::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject AboutGui::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_AboutGui,
      qt_meta_data_AboutGui, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AboutGui::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AboutGui::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AboutGui::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AboutGui))
        return static_cast<void*>(const_cast< AboutGui*>(this));
    return QWidget::qt_metacast(_clname);
}

int AboutGui::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_WebViewErrCheck[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      20,   17,   16,   16, 0x0a,
      40,   16,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_WebViewErrCheck[] = {
    "WebViewErrCheck\0\0ok\0LoadingResult(bool)\0"
    "LoadInitialPage()\0"
};

void WebViewErrCheck::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        WebViewErrCheck *_t = static_cast<WebViewErrCheck *>(_o);
        switch (_id) {
        case 0: _t->LoadingResult((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->LoadInitialPage(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData WebViewErrCheck::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject WebViewErrCheck::staticMetaObject = {
    { &QWebView::staticMetaObject, qt_meta_stringdata_WebViewErrCheck,
      qt_meta_data_WebViewErrCheck, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &WebViewErrCheck::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *WebViewErrCheck::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *WebViewErrCheck::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_WebViewErrCheck))
        return static_cast<void*>(const_cast< WebViewErrCheck*>(this));
    return QWebView::qt_metacast(_clname);
}

int WebViewErrCheck::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWebView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
