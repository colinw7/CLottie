/****************************************************************************
** Meta object code from reading C++ file 'CQLottie.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../CQLottie.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CQLottie.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CQLottie_t {
    QByteArrayData data[7];
    char stringdata0[56];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CQLottie_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CQLottie_t qt_meta_stringdata_CQLottie = {
    {
QT_MOC_LITERAL(0, 0, 8), // "CQLottie"
QT_MOC_LITERAL(1, 9, 8), // "loadSlot"
QT_MOC_LITERAL(2, 18, 0), // ""
QT_MOC_LITERAL(3, 19, 8), // "playSlot"
QT_MOC_LITERAL(4, 28, 9), // "pauseSlot"
QT_MOC_LITERAL(5, 38, 8), // "stepSlot"
QT_MOC_LITERAL(6, 47, 8) // "tickSlot"

    },
    "CQLottie\0loadSlot\0\0playSlot\0pauseSlot\0"
    "stepSlot\0tickSlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CQLottie[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x0a /* Public */,
       3,    0,   40,    2, 0x0a /* Public */,
       4,    0,   41,    2, 0x0a /* Public */,
       5,    0,   42,    2, 0x0a /* Public */,
       6,    0,   43,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CQLottie::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CQLottie *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->loadSlot(); break;
        case 1: _t->playSlot(); break;
        case 2: _t->pauseSlot(); break;
        case 3: _t->stepSlot(); break;
        case 4: _t->tickSlot(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject CQLottie::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CQLottie.data,
    qt_meta_data_CQLottie,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CQLottie::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CQLottie::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CQLottie.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CQLottie::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
struct qt_meta_stringdata_CQLottieCanvas_t {
    QByteArrayData data[1];
    char stringdata0[15];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CQLottieCanvas_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CQLottieCanvas_t qt_meta_stringdata_CQLottieCanvas = {
    {
QT_MOC_LITERAL(0, 0, 14) // "CQLottieCanvas"

    },
    "CQLottieCanvas"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CQLottieCanvas[] = {

 // content:
       8,       // revision
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

void CQLottieCanvas::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject CQLottieCanvas::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CQLottieCanvas.data,
    qt_meta_data_CQLottieCanvas,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CQLottieCanvas::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CQLottieCanvas::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CQLottieCanvas.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CQLottieCanvas::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_CQLottieToolBar_t {
    QByteArrayData data[6];
    char stringdata0[54];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CQLottieToolBar_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CQLottieToolBar_t qt_meta_stringdata_CQLottieToolBar = {
    {
QT_MOC_LITERAL(0, 0, 15), // "CQLottieToolBar"
QT_MOC_LITERAL(1, 16, 8), // "loadSlot"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 8), // "playSlot"
QT_MOC_LITERAL(4, 35, 9), // "pauseSlot"
QT_MOC_LITERAL(5, 45, 8) // "stepSlot"

    },
    "CQLottieToolBar\0loadSlot\0\0playSlot\0"
    "pauseSlot\0stepSlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CQLottieToolBar[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x08 /* Private */,
       3,    0,   35,    2, 0x08 /* Private */,
       4,    0,   36,    2, 0x08 /* Private */,
       5,    0,   37,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CQLottieToolBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CQLottieToolBar *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->loadSlot(); break;
        case 1: _t->playSlot(); break;
        case 2: _t->pauseSlot(); break;
        case 3: _t->stepSlot(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject CQLottieToolBar::staticMetaObject = { {
    QMetaObject::SuperData::link<QFrame::staticMetaObject>(),
    qt_meta_stringdata_CQLottieToolBar.data,
    qt_meta_data_CQLottieToolBar,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CQLottieToolBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CQLottieToolBar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CQLottieToolBar.stringdata0))
        return static_cast<void*>(this);
    return QFrame::qt_metacast(_clname);
}

int CQLottieToolBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
struct qt_meta_stringdata_CQLottieSettings_t {
    QByteArrayData data[6];
    char stringdata0[80];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CQLottieSettings_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CQLottieSettings_t qt_meta_stringdata_CQLottieSettings = {
    {
QT_MOC_LITERAL(0, 0, 16), // "CQLottieSettings"
QT_MOC_LITERAL(1, 17, 10), // "bgFillSlot"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 16), // "selectedFillSlot"
QT_MOC_LITERAL(4, 46, 18), // "selectedStrokeSlot"
QT_MOC_LITERAL(5, 65, 14) // "bboxStrokeSlot"

    },
    "CQLottieSettings\0bgFillSlot\0\0"
    "selectedFillSlot\0selectedStrokeSlot\0"
    "bboxStrokeSlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CQLottieSettings[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x08 /* Private */,
       3,    1,   37,    2, 0x08 /* Private */,
       4,    1,   40,    2, 0x08 /* Private */,
       5,    1,   43,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QColor,    2,
    QMetaType::Void, QMetaType::QColor,    2,
    QMetaType::Void, QMetaType::QColor,    2,
    QMetaType::Void, QMetaType::QColor,    2,

       0        // eod
};

void CQLottieSettings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CQLottieSettings *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->bgFillSlot((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        case 1: _t->selectedFillSlot((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        case 2: _t->selectedStrokeSlot((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        case 3: _t->bboxStrokeSlot((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CQLottieSettings::staticMetaObject = { {
    QMetaObject::SuperData::link<QFrame::staticMetaObject>(),
    qt_meta_stringdata_CQLottieSettings.data,
    qt_meta_data_CQLottieSettings,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CQLottieSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CQLottieSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CQLottieSettings.stringdata0))
        return static_cast<void*>(this);
    return QFrame::qt_metacast(_clname);
}

int CQLottieSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
