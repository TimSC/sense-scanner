/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created: Wed Dec 5 15:39:10 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../QtMedia/mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SourcesList[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   13,   12,   12, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_SourcesList[] = {
    "SourcesList\0\0current\0UpdateSources(QModelIndex)\0"
};

void SourcesList::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SourcesList *_t = static_cast<SourcesList *>(_o);
        switch (_id) {
        case 0: _t->UpdateSources((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData SourcesList::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SourcesList::staticMetaObject = {
    { &QListView::staticMetaObject, qt_meta_stringdata_SourcesList,
      qt_meta_data_SourcesList, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SourcesList::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SourcesList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SourcesList::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SourcesList))
        return static_cast<void*>(const_cast< SourcesList*>(this));
    return QListView::qt_metacast(_clname);
}

int SourcesList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QListView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void SourcesList::UpdateSources(const QModelIndex _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_MainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,
      26,   11,   11,   11, 0x0a,
      40,   11,   11,   11, 0x0a,
      55,   49,   11,   11, 0x0a,
      80,   11,   11,   11, 0x0a,
     104,   11,   11,   11, 0x0a,
     119,   11,   11,   11, 0x0a,
     135,   11,   11,   11, 0x0a,
     151,   11,   11,   11, 0x0a,
     177,  169,   11,   11, 0x0a,
     212,   11,   11,   11, 0x0a,
     229,   11,   11,   11, 0x0a,
     251,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0ImportVideo()\0RemoveVideo()\0"
    "Update()\0event\0closeEvent(QCloseEvent*)\0"
    "RegenerateSourcesList()\0NewWorkspace()\0"
    "LoadWorkspace()\0SaveWorkspace()\0"
    "SaveAsWorkspace()\0current\0"
    "SelectedSourceChanged(QModelIndex)\0"
    "ShutdownSaveAs()\0ShutdownWithoutSave()\0"
    "ShutdownCancel()\0"
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->ImportVideo(); break;
        case 1: _t->RemoveVideo(); break;
        case 2: _t->Update(); break;
        case 3: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        case 4: _t->RegenerateSourcesList(); break;
        case 5: _t->NewWorkspace(); break;
        case 6: _t->LoadWorkspace(); break;
        case 7: _t->SaveWorkspace(); break;
        case 8: _t->SaveAsWorkspace(); break;
        case 9: _t->SelectedSourceChanged((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 10: _t->ShutdownSaveAs(); break;
        case 11: _t->ShutdownWithoutSave(); break;
        case 12: _t->ShutdownCancel(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
