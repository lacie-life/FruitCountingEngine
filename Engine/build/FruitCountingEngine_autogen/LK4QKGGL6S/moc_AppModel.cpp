/****************************************************************************
** Meta object code from reading C++ file 'AppModel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../Engine/include/AppModel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AppModel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AppModel_t {
    QByteArrayData data[14];
    char stringdata0[132];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AppModel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AppModel_t qt_meta_stringdata_AppModel = {
    {
QT_MOC_LITERAL(0, 0, 8), // "AppModel"
QT_MOC_LITERAL(1, 9, 10), // "imageReady"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 6), // "pixmap"
QT_MOC_LITERAL(4, 28, 12), // "processImage"
QT_MOC_LITERAL(5, 41, 7), // "cv::Mat"
QT_MOC_LITERAL(6, 49, 5), // "frame"
QT_MOC_LITERAL(7, 55, 8), // "setState"
QT_MOC_LITERAL(8, 64, 9), // "APP_STATE"
QT_MOC_LITERAL(9, 74, 5), // "state"
QT_MOC_LITERAL(10, 80, 10), // "NONE_STATE"
QT_MOC_LITERAL(11, 91, 15), // "DETECTING_STATE"
QT_MOC_LITERAL(12, 107, 14), // "COUNTING_STATE"
QT_MOC_LITERAL(13, 122, 9) // "END_STATE"

    },
    "AppModel\0imageReady\0\0pixmap\0processImage\0"
    "cv::Mat\0frame\0setState\0APP_STATE\0state\0"
    "NONE_STATE\0DETECTING_STATE\0COUNTING_STATE\0"
    "END_STATE"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AppModel[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       1,   38, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   32,    2, 0x0a /* Public */,
       7,    1,   35,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QPixmap,    3,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 8,    9,

 // enums: name, flags, count, data
       8, 0x0,    4,   42,

 // enum data: key, value
      10, uint(AppModel::NONE_STATE),
      11, uint(AppModel::DETECTING_STATE),
      12, uint(AppModel::COUNTING_STATE),
      13, uint(AppModel::END_STATE),

       0        // eod
};

void AppModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        AppModel *_t = static_cast<AppModel *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->imageReady((*reinterpret_cast< QPixmap(*)>(_a[1]))); break;
        case 1: _t->processImage((*reinterpret_cast< cv::Mat(*)>(_a[1]))); break;
        case 2: _t->setState((*reinterpret_cast< APP_STATE(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (AppModel::*_t)(QPixmap );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AppModel::imageReady)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject AppModel::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_AppModel.data,
      qt_meta_data_AppModel,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *AppModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AppModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AppModel.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AppModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void AppModel::imageReady(QPixmap _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
