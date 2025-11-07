/****************************************************************************
** Meta object code from reading C++ file 'CQLottieTree.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../CQLottieTree.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CQLottieTree.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CQLottieTree_t {
    QByteArrayData data[16];
    char stringdata0[193];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CQLottieTree_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CQLottieTree_t qt_meta_stringdata_CQLottieTree = {
    {
QT_MOC_LITERAL(0, 0, 12), // "CQLottieTree"
QT_MOC_LITERAL(1, 13, 15), // "itemClickedSlot"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(4, 47, 4), // "item"
QT_MOC_LITERAL(5, 52, 6), // "column"
QT_MOC_LITERAL(6, 59, 16), // "itemSelectedSlot"
QT_MOC_LITERAL(7, 76, 21), // "customContextMenuSlot"
QT_MOC_LITERAL(8, 98, 3), // "pos"
QT_MOC_LITERAL(9, 102, 13), // "expandAllSlot"
QT_MOC_LITERAL(10, 116, 15), // "collapseAllSlot"
QT_MOC_LITERAL(11, 132, 8), // "bboxSlot"
QT_MOC_LITERAL(12, 141, 13), // "transformSlot"
QT_MOC_LITERAL(13, 155, 17), // "hierTransformSlot"
QT_MOC_LITERAL(14, 173, 9), // "imageSlot"
QT_MOC_LITERAL(15, 183, 9) // "printSlot"

    },
    "CQLottieTree\0itemClickedSlot\0\0"
    "QTreeWidgetItem*\0item\0column\0"
    "itemSelectedSlot\0customContextMenuSlot\0"
    "pos\0expandAllSlot\0collapseAllSlot\0"
    "bboxSlot\0transformSlot\0hierTransformSlot\0"
    "imageSlot\0printSlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CQLottieTree[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   64,    2, 0x08 /* Private */,
       6,    2,   69,    2, 0x08 /* Private */,
       7,    1,   74,    2, 0x08 /* Private */,
       9,    0,   77,    2, 0x08 /* Private */,
      10,    0,   78,    2, 0x08 /* Private */,
      11,    0,   79,    2, 0x08 /* Private */,
      12,    0,   80,    2, 0x08 /* Private */,
      13,    0,   81,    2, 0x08 /* Private */,
      14,    0,   82,    2, 0x08 /* Private */,
      15,    0,   83,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    2,    2,
    QMetaType::Void, QMetaType::QPoint,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CQLottieTree::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CQLottieTree *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->itemClickedSlot((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->itemSelectedSlot((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QTreeWidgetItem*(*)>(_a[2]))); break;
        case 2: _t->customContextMenuSlot((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 3: _t->expandAllSlot(); break;
        case 4: _t->collapseAllSlot(); break;
        case 5: _t->bboxSlot(); break;
        case 6: _t->transformSlot(); break;
        case 7: _t->hierTransformSlot(); break;
        case 8: _t->imageSlot(); break;
        case 9: _t->printSlot(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CQLottieTree::staticMetaObject = { {
    QMetaObject::SuperData::link<QFrame::staticMetaObject>(),
    qt_meta_stringdata_CQLottieTree.data,
    qt_meta_data_CQLottieTree,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CQLottieTree::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CQLottieTree::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CQLottieTree.stringdata0))
        return static_cast<void*>(this);
    return QFrame::qt_metacast(_clname);
}

int CQLottieTree::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
struct qt_meta_stringdata_CQLottieTreeWidget_t {
    QByteArrayData data[1];
    char stringdata0[19];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CQLottieTreeWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CQLottieTreeWidget_t qt_meta_stringdata_CQLottieTreeWidget = {
    {
QT_MOC_LITERAL(0, 0, 18) // "CQLottieTreeWidget"

    },
    "CQLottieTreeWidget"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CQLottieTreeWidget[] = {

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

void CQLottieTreeWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject CQLottieTreeWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QTreeWidget::staticMetaObject>(),
    qt_meta_stringdata_CQLottieTreeWidget.data,
    qt_meta_data_CQLottieTreeWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CQLottieTreeWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CQLottieTreeWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CQLottieTreeWidget.stringdata0))
        return static_cast<void*>(this);
    return QTreeWidget::qt_metacast(_clname);
}

int CQLottieTreeWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeWidget::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_CQLottieTreeDelegate_t {
    QByteArrayData data[1];
    char stringdata0[21];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CQLottieTreeDelegate_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CQLottieTreeDelegate_t qt_meta_stringdata_CQLottieTreeDelegate = {
    {
QT_MOC_LITERAL(0, 0, 20) // "CQLottieTreeDelegate"

    },
    "CQLottieTreeDelegate"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CQLottieTreeDelegate[] = {

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

void CQLottieTreeDelegate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject CQLottieTreeDelegate::staticMetaObject = { {
    QMetaObject::SuperData::link<QItemDelegate::staticMetaObject>(),
    qt_meta_stringdata_CQLottieTreeDelegate.data,
    qt_meta_data_CQLottieTreeDelegate,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CQLottieTreeDelegate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CQLottieTreeDelegate::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CQLottieTreeDelegate.stringdata0))
        return static_cast<void*>(this);
    return QItemDelegate::qt_metacast(_clname);
}

int CQLottieTreeDelegate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QItemDelegate::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_CQLottieObjectTree_t {
    QByteArrayData data[15];
    char stringdata0[169];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CQLottieObjectTree_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CQLottieObjectTree_t qt_meta_stringdata_CQLottieObjectTree = {
    {
QT_MOC_LITERAL(0, 0, 18), // "CQLottieObjectTree"
QT_MOC_LITERAL(1, 19, 15), // "itemClickedSlot"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(4, 53, 4), // "item"
QT_MOC_LITERAL(5, 58, 6), // "column"
QT_MOC_LITERAL(6, 65, 16), // "itemSelectedSlot"
QT_MOC_LITERAL(7, 82, 21), // "customContextMenuSlot"
QT_MOC_LITERAL(8, 104, 3), // "pos"
QT_MOC_LITERAL(9, 108, 9), // "expandAll"
QT_MOC_LITERAL(10, 118, 11), // "QModelIndex"
QT_MOC_LITERAL(11, 130, 3), // "ind"
QT_MOC_LITERAL(12, 134, 11), // "collapseAll"
QT_MOC_LITERAL(13, 146, 9), // "printSlot"
QT_MOC_LITERAL(14, 156, 12) // "printAllSlot"

    },
    "CQLottieObjectTree\0itemClickedSlot\0\0"
    "QTreeWidgetItem*\0item\0column\0"
    "itemSelectedSlot\0customContextMenuSlot\0"
    "pos\0expandAll\0QModelIndex\0ind\0collapseAll\0"
    "printSlot\0printAllSlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CQLottieObjectTree[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   59,    2, 0x08 /* Private */,
       6,    2,   64,    2, 0x08 /* Private */,
       7,    1,   69,    2, 0x08 /* Private */,
       9,    1,   72,    2, 0x08 /* Private */,
       9,    0,   75,    2, 0x28 /* Private | MethodCloned */,
      12,    1,   76,    2, 0x08 /* Private */,
      12,    0,   79,    2, 0x28 /* Private | MethodCloned */,
      13,    0,   80,    2, 0x08 /* Private */,
      14,    0,   81,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3,    2,    2,
    QMetaType::Void, QMetaType::QPoint,    8,
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CQLottieObjectTree::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CQLottieObjectTree *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->itemClickedSlot((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->itemSelectedSlot((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QTreeWidgetItem*(*)>(_a[2]))); break;
        case 2: _t->customContextMenuSlot((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 3: _t->expandAll((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 4: _t->expandAll(); break;
        case 5: _t->collapseAll((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 6: _t->collapseAll(); break;
        case 7: _t->printSlot(); break;
        case 8: _t->printAllSlot(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CQLottieObjectTree::staticMetaObject = { {
    QMetaObject::SuperData::link<QFrame::staticMetaObject>(),
    qt_meta_stringdata_CQLottieObjectTree.data,
    qt_meta_data_CQLottieObjectTree,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CQLottieObjectTree::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CQLottieObjectTree::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CQLottieObjectTree.stringdata0))
        return static_cast<void*>(this);
    return QFrame::qt_metacast(_clname);
}

int CQLottieObjectTree::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}
struct qt_meta_stringdata_CQLottieObjectTreeWidget_t {
    QByteArrayData data[1];
    char stringdata0[25];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CQLottieObjectTreeWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CQLottieObjectTreeWidget_t qt_meta_stringdata_CQLottieObjectTreeWidget = {
    {
QT_MOC_LITERAL(0, 0, 24) // "CQLottieObjectTreeWidget"

    },
    "CQLottieObjectTreeWidget"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CQLottieObjectTreeWidget[] = {

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

void CQLottieObjectTreeWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject CQLottieObjectTreeWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QTreeWidget::staticMetaObject>(),
    qt_meta_stringdata_CQLottieObjectTreeWidget.data,
    qt_meta_data_CQLottieObjectTreeWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CQLottieObjectTreeWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CQLottieObjectTreeWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CQLottieObjectTreeWidget.stringdata0))
        return static_cast<void*>(this);
    return QTreeWidget::qt_metacast(_clname);
}

int CQLottieObjectTreeWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeWidget::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_CQLottieObjectTreeDelegate_t {
    QByteArrayData data[3];
    char stringdata0[40];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CQLottieObjectTreeDelegate_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CQLottieObjectTreeDelegate_t qt_meta_stringdata_CQLottieObjectTreeDelegate = {
    {
QT_MOC_LITERAL(0, 0, 26), // "CQLottieObjectTreeDelegate"
QT_MOC_LITERAL(1, 27, 11), // "updateValue"
QT_MOC_LITERAL(2, 39, 0) // ""

    },
    "CQLottieObjectTreeDelegate\0updateValue\0"
    ""
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CQLottieObjectTreeDelegate[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void CQLottieObjectTreeDelegate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CQLottieObjectTreeDelegate *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->updateValue(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject CQLottieObjectTreeDelegate::staticMetaObject = { {
    QMetaObject::SuperData::link<QItemDelegate::staticMetaObject>(),
    qt_meta_stringdata_CQLottieObjectTreeDelegate.data,
    qt_meta_data_CQLottieObjectTreeDelegate,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CQLottieObjectTreeDelegate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CQLottieObjectTreeDelegate::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CQLottieObjectTreeDelegate.stringdata0))
        return static_cast<void*>(this);
    return QItemDelegate::qt_metacast(_clname);
}

int CQLottieObjectTreeDelegate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QItemDelegate::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
