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
    QByteArrayData data[11];
    char stringdata0[95];
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
QT_MOC_LITERAL(6, 47, 8), // "zoomFull"
QT_MOC_LITERAL(7, 56, 15), // "setShowTimeLine"
QT_MOC_LITERAL(8, 72, 1), // "b"
QT_MOC_LITERAL(9, 74, 11), // "setShowPath"
QT_MOC_LITERAL(10, 86, 8) // "tickSlot"

    },
    "CQLottie\0loadSlot\0\0playSlot\0pauseSlot\0"
    "stepSlot\0zoomFull\0setShowTimeLine\0b\0"
    "setShowPath\0tickSlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CQLottie[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x0a /* Public */,
       3,    0,   55,    2, 0x0a /* Public */,
       4,    0,   56,    2, 0x0a /* Public */,
       5,    0,   57,    2, 0x0a /* Public */,
       6,    0,   58,    2, 0x0a /* Public */,
       7,    1,   59,    2, 0x0a /* Public */,
       9,    1,   62,    2, 0x0a /* Public */,
      10,    0,   65,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    8,
    QMetaType::Void, QMetaType::Bool,    8,
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
        case 4: _t->zoomFull(); break;
        case 5: _t->setShowTimeLine((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->setShowPath((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->tickSlot(); break;
        default: ;
        }
    }
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
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
