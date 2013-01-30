/****************************************************************************
** Meta object code from reading C++ file 'sourcealggui.h'
**
** Created: Wed Jan 30 21:21:55 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtMedia/sourcealggui.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sourcealggui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SourceAlgGui[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x0a,
      28,   13,   13,   13, 0x0a,
      42,   13,   13,   13, 0x0a,
      62,   13,   13,   13, 0x0a,
      82,   13,   13,   13, 0x0a,
     104,   13,   13,   13, 0x0a,
     124,   13,   13,   13, 0x0a,
     155,  147,   13,   13, 0x0a,
     190,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_SourceAlgGui[] = {
    "SourceAlgGui\0\0ImportVideo()\0RemoveVideo()\0"
    "TrainModelPressed()\0ApplyModelPressed()\0"
    "PauseProcessPressed()\0RunProcessPressed()\0"
    "RemoveProcessPressed()\0current\0"
    "SelectedSourceChanged(QModelIndex)\0"
    "DeselectCurrentSource()\0"
};

void SourceAlgGui::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SourceAlgGui *_t = static_cast<SourceAlgGui *>(_o);
        switch (_id) {
        case 0: _t->ImportVideo(); break;
        case 1: _t->RemoveVideo(); break;
        case 2: _t->TrainModelPressed(); break;
        case 3: _t->ApplyModelPressed(); break;
        case 4: _t->PauseProcessPressed(); break;
        case 5: _t->RunProcessPressed(); break;
        case 6: _t->RemoveProcessPressed(); break;
        case 7: _t->SelectedSourceChanged((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 8: _t->DeselectCurrentSource(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData SourceAlgGui::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SourceAlgGui::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_SourceAlgGui,
      qt_meta_data_SourceAlgGui, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SourceAlgGui::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SourceAlgGui::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SourceAlgGui::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SourceAlgGui))
        return static_cast<void*>(const_cast< SourceAlgGui*>(this));
    return QWidget::qt_metacast(_clname);
}

int SourceAlgGui::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
