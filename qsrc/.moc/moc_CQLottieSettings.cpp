/****************************************************************************
** Meta object code from reading C++ file 'CQLottieSettings.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../CQLottieSettings.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CQLottieSettings.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CQLottieSettings_t {
    QByteArrayData data[9];
    char stringdata0[123];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CQLottieSettings_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CQLottieSettings_t qt_meta_stringdata_CQLottieSettings = {
    {
QT_MOC_LITERAL(0, 0, 16), // "CQLottieSettings"
QT_MOC_LITERAL(1, 17, 14), // "equalScaleSlot"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 10), // "bgFillSlot"
QT_MOC_LITERAL(4, 44, 14), // "showSelectSlot"
QT_MOC_LITERAL(5, 59, 16), // "selectedFillSlot"
QT_MOC_LITERAL(6, 76, 18), // "selectedStrokeSlot"
QT_MOC_LITERAL(7, 95, 12), // "showBBoxSlot"
QT_MOC_LITERAL(8, 108, 14) // "bboxStrokeSlot"

    },
    "CQLottieSettings\0equalScaleSlot\0\0"
    "bgFillSlot\0showSelectSlot\0selectedFillSlot\0"
    "selectedStrokeSlot\0showBBoxSlot\0"
    "bboxStrokeSlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CQLottieSettings[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x08 /* Private */,
       3,    1,   52,    2, 0x08 /* Private */,
       4,    1,   55,    2, 0x08 /* Private */,
       5,    1,   58,    2, 0x08 /* Private */,
       6,    1,   61,    2, 0x08 /* Private */,
       7,    1,   64,    2, 0x08 /* Private */,
       8,    1,   67,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QColor,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QColor,    2,
    QMetaType::Void, QMetaType::QColor,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QColor,    2,

       0        // eod
};

void CQLottieSettings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CQLottieSettings *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->equalScaleSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->bgFillSlot((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        case 2: _t->showSelectSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->selectedFillSlot((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        case 4: _t->selectedStrokeSlot((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        case 5: _t->showBBoxSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->bboxStrokeSlot((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
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
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
