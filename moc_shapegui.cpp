/****************************************************************************
** Meta object code from reading C++ file 'shapegui.h'
**
** Created: Mon Apr 8 19:27:00 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtMedia/shapegui.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'shapegui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QListViewWithChanges[] = {

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

static const char qt_meta_stringdata_QListViewWithChanges[] = {
    "QListViewWithChanges\0"
};

void QListViewWithChanges::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData QListViewWithChanges::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QListViewWithChanges::staticMetaObject = {
    { &QListView::staticMetaObject, qt_meta_stringdata_QListViewWithChanges,
      qt_meta_data_QListViewWithChanges, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QListViewWithChanges::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QListViewWithChanges::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QListViewWithChanges::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QListViewWithChanges))
        return static_cast<void*>(const_cast< QListViewWithChanges*>(this));
    return QListView::qt_metacast(_clname);
}

int QListViewWithChanges::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QListView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_ShapeGui[] = {

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
      10,    9,    9,    9, 0x0a,
      29,    9,    9,    9, 0x0a,
      48,    9,    9,    9, 0x0a,
      75,    9,   67,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ShapeGui[] = {
    "ShapeGui\0\0UsePresetPressed()\0"
    "UseCustomPressed()\0LoadShapePressed()\0"
    "QString\0GetCustomFilename()\0"
};

void ShapeGui::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ShapeGui *_t = static_cast<ShapeGui *>(_o);
        switch (_id) {
        case 0: _t->UsePresetPressed(); break;
        case 1: _t->UseCustomPressed(); break;
        case 2: _t->LoadShapePressed(); break;
        case 3: { QString _r = _t->GetCustomFilename();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ShapeGui::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ShapeGui::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_ShapeGui,
      qt_meta_data_ShapeGui, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ShapeGui::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ShapeGui::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ShapeGui::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ShapeGui))
        return static_cast<void*>(const_cast< ShapeGui*>(this));
    return QDialog::qt_metacast(_clname);
}

int ShapeGui::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
