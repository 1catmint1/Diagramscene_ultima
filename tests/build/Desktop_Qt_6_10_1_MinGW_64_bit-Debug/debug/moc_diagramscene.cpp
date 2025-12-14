/****************************************************************************
** Meta object code from reading C++ file 'diagramscene.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../diagramscene.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'diagramscene.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN12DiagramSceneE_t {};
} // unnamed namespace

template <> constexpr inline auto DiagramScene::qt_create_metaobjectdata<qt_meta_tag_ZN12DiagramSceneE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DiagramScene",
        "itemInserted",
        "",
        "DiagramItem*",
        "item",
        "textInserted",
        "QGraphicsTextItem*",
        "itemSelected",
        "QGraphicsItem*",
        "pathInserted",
        "DiagramPath*",
        "path",
        "setMode",
        "Mode",
        "mode",
        "setItemType",
        "DiagramItem::DiagramType",
        "type",
        "editorLostFocus",
        "DiagramTextItem*",
        "createItem",
        "createTextItem"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'itemInserted'
        QtMocHelpers::SignalData<void(DiagramItem *)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'textInserted'
        QtMocHelpers::SignalData<void(QGraphicsTextItem *)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 4 },
        }}),
        // Signal 'itemSelected'
        QtMocHelpers::SignalData<void(QGraphicsItem *)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 4 },
        }}),
        // Signal 'pathInserted'
        QtMocHelpers::SignalData<void(DiagramPath *)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 },
        }}),
        // Slot 'setMode'
        QtMocHelpers::SlotData<void(enum Mode)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 13, 14 },
        }}),
        // Slot 'setItemType'
        QtMocHelpers::SlotData<void(DiagramItem::DiagramType)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 16, 17 },
        }}),
        // Slot 'editorLostFocus'
        QtMocHelpers::SlotData<void(DiagramTextItem *)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 19, 4 },
        }}),
        // Slot 'createItem'
        QtMocHelpers::SlotData<QGraphicsItem *(int)>(20, 2, QMC::AccessPublic, 0x80000000 | 8, {{
            { QMetaType::Int, 17 },
        }}),
        // Slot 'createTextItem'
        QtMocHelpers::SlotData<QGraphicsTextItem *()>(21, 2, QMC::AccessPublic, 0x80000000 | 6),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DiagramScene, qt_meta_tag_ZN12DiagramSceneE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DiagramScene::staticMetaObject = { {
    QMetaObject::SuperData::link<QGraphicsScene::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12DiagramSceneE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12DiagramSceneE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12DiagramSceneE_t>.metaTypes,
    nullptr
} };

void DiagramScene::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DiagramScene *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->itemInserted((*reinterpret_cast<std::add_pointer_t<DiagramItem*>>(_a[1]))); break;
        case 1: _t->textInserted((*reinterpret_cast<std::add_pointer_t<QGraphicsTextItem*>>(_a[1]))); break;
        case 2: _t->itemSelected((*reinterpret_cast<std::add_pointer_t<QGraphicsItem*>>(_a[1]))); break;
        case 3: _t->pathInserted((*reinterpret_cast<std::add_pointer_t<DiagramPath*>>(_a[1]))); break;
        case 4: _t->setMode((*reinterpret_cast<std::add_pointer_t<enum Mode>>(_a[1]))); break;
        case 5: _t->setItemType((*reinterpret_cast<std::add_pointer_t<DiagramItem::DiagramType>>(_a[1]))); break;
        case 6: _t->editorLostFocus((*reinterpret_cast<std::add_pointer_t<DiagramTextItem*>>(_a[1]))); break;
        case 7: { QGraphicsItem* _r = _t->createItem((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QGraphicsItem**>(_a[0]) = std::move(_r); }  break;
        case 8: { QGraphicsTextItem* _r = _t->createTextItem();
            if (_a[0]) *reinterpret_cast<QGraphicsTextItem**>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QGraphicsTextItem* >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QGraphicsItem* >(); break;
            }
            break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< DiagramTextItem* >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DiagramScene::*)(DiagramItem * )>(_a, &DiagramScene::itemInserted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DiagramScene::*)(QGraphicsTextItem * )>(_a, &DiagramScene::textInserted, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DiagramScene::*)(QGraphicsItem * )>(_a, &DiagramScene::itemSelected, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (DiagramScene::*)(DiagramPath * )>(_a, &DiagramScene::pathInserted, 3))
            return;
    }
}

const QMetaObject *DiagramScene::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DiagramScene::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12DiagramSceneE_t>.strings))
        return static_cast<void*>(this);
    return QGraphicsScene::qt_metacast(_clname);
}

int DiagramScene::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsScene::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void DiagramScene::itemInserted(DiagramItem * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void DiagramScene::textInserted(QGraphicsTextItem * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void DiagramScene::itemSelected(QGraphicsItem * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void DiagramScene::pathInserted(DiagramPath * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
QT_WARNING_POP
