/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../mainwindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "copyItems",
        "",
        "pasteItems",
        "QPointF",
        "scenePos",
        "cutItems",
        "backgroundButtonGroupClicked",
        "QAbstractButton*",
        "button",
        "buttonGroupClicked",
        "deleteItem",
        "pointerGroupClicked",
        "bringToFront",
        "sendToBack",
        "itemInserted",
        "DiagramItem*",
        "item",
        "textInserted",
        "QGraphicsTextItem*",
        "currentFontChanged",
        "QFont",
        "font",
        "fontSizeChanged",
        "size",
        "sceneScaleChanged",
        "scale",
        "textButtonTriggered",
        "fillButtonTriggered",
        "lineButtonTriggered",
        "handleFontChange",
        "itemSelected",
        "QGraphicsItem*",
        "about",
        "saveSceneAsImage",
        "closeEvent",
        "QCloseEvent*",
        "event",
        "backgroundChanged",
        "index",
        "newScene",
        "sceneymChanged",
        "closeScene",
        "showContextMenu",
        "QPoint",
        "pos",
        "pasteItemsFromMenu",
        "sceneChanged",
        "openFindReplaceDialog",
        "handleFindText",
        "text",
        "handleReplaceText",
        "findText",
        "replaceText",
        "handleReplaceAllText",
        "combination",
        "cancelCombination",
        "savefile",
        "saveSaveFilePath",
        "filePath",
        "changebackground",
        "saveSceneAsImageOrSvg",
        "loadSaveFilePath",
        "loadfile",
        "getStructList",
        "QList<WriteDiagramItem*>",
        "getStructList1",
        "QList<WriteDiagramPath*>",
        "savefilestack",
        "loadfilestack",
        "str",
        "autoCleanStack",
        "undo",
        "redo"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'copyItems'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'pasteItems'
        QtMocHelpers::SlotData<void(const QPointF &)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Slot 'cutItems'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'backgroundButtonGroupClicked'
        QtMocHelpers::SlotData<void(QAbstractButton *)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Slot 'buttonGroupClicked'
        QtMocHelpers::SlotData<void(QAbstractButton *)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Slot 'deleteItem'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'pointerGroupClicked'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'bringToFront'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'sendToBack'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'itemInserted'
        QtMocHelpers::SlotData<void(DiagramItem *)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 16, 17 },
        }}),
        // Slot 'textInserted'
        QtMocHelpers::SlotData<void(QGraphicsTextItem *)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 19, 17 },
        }}),
        // Slot 'currentFontChanged'
        QtMocHelpers::SlotData<void(const QFont &)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 21, 22 },
        }}),
        // Slot 'fontSizeChanged'
        QtMocHelpers::SlotData<void(const QString &)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 24 },
        }}),
        // Slot 'sceneScaleChanged'
        QtMocHelpers::SlotData<void(const QString &)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 26 },
        }}),
        // Slot 'textButtonTriggered'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'fillButtonTriggered'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'lineButtonTriggered'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'handleFontChange'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'itemSelected'
        QtMocHelpers::SlotData<void(QGraphicsItem *)>(31, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 32, 17 },
        }}),
        // Slot 'about'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'saveSceneAsImage'
        QtMocHelpers::SlotData<bool()>(34, 2, QMC::AccessPrivate, QMetaType::Bool),
        // Slot 'closeEvent'
        QtMocHelpers::SlotData<void(QCloseEvent *)>(35, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 36, 37 },
        }}),
        // Slot 'backgroundChanged'
        QtMocHelpers::SlotData<void(int)>(38, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 39 },
        }}),
        // Slot 'newScene'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'sceneymChanged'
        QtMocHelpers::SlotData<void()>(41, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'closeScene'
        QtMocHelpers::SlotData<void(int)>(42, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 39 },
        }}),
        // Slot 'showContextMenu'
        QtMocHelpers::SlotData<void(const QPoint &)>(43, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 44, 45 },
        }}),
        // Slot 'pasteItemsFromMenu'
        QtMocHelpers::SlotData<void()>(46, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'sceneChanged'
        QtMocHelpers::SlotData<void()>(47, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'openFindReplaceDialog'
        QtMocHelpers::SlotData<void()>(48, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'handleFindText'
        QtMocHelpers::SlotData<void(const QString &)>(49, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 50 },
        }}),
        // Slot 'handleReplaceText'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(51, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 52 }, { QMetaType::QString, 53 },
        }}),
        // Slot 'handleReplaceAllText'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(54, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 52 }, { QMetaType::QString, 53 },
        }}),
        // Slot 'combination'
        QtMocHelpers::SlotData<void()>(55, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'cancelCombination'
        QtMocHelpers::SlotData<void()>(56, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'savefile'
        QtMocHelpers::SlotData<void()>(57, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'saveSaveFilePath'
        QtMocHelpers::SlotData<void(const QString &)>(58, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 59 },
        }}),
        // Slot 'changebackground'
        QtMocHelpers::SlotData<void()>(60, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'saveSceneAsImageOrSvg'
        QtMocHelpers::SlotData<bool()>(61, 2, QMC::AccessPrivate, QMetaType::Bool),
        // Slot 'loadSaveFilePath'
        QtMocHelpers::SlotData<QString()>(62, 2, QMC::AccessPrivate, QMetaType::QString),
        // Slot 'loadfile'
        QtMocHelpers::SlotData<void()>(63, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'getStructList'
        QtMocHelpers::SlotData<QList<struct WriteDiagramItem*>()>(64, 2, QMC::AccessPrivate, 0x80000000 | 65),
        // Slot 'getStructList1'
        QtMocHelpers::SlotData<QList<struct WriteDiagramPath*>()>(66, 2, QMC::AccessPrivate, 0x80000000 | 67),
        // Slot 'savefilestack'
        QtMocHelpers::SlotData<QString()>(68, 2, QMC::AccessPrivate, QMetaType::QString),
        // Slot 'loadfilestack'
        QtMocHelpers::SlotData<void(QString)>(69, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 70 },
        }}),
        // Slot 'autoCleanStack'
        QtMocHelpers::SlotData<void()>(71, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'undo'
        QtMocHelpers::SlotData<void()>(72, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'redo'
        QtMocHelpers::SlotData<void()>(73, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->copyItems(); break;
        case 1: _t->pasteItems((*reinterpret_cast<std::add_pointer_t<QPointF>>(_a[1]))); break;
        case 2: _t->cutItems(); break;
        case 3: _t->backgroundButtonGroupClicked((*reinterpret_cast<std::add_pointer_t<QAbstractButton*>>(_a[1]))); break;
        case 4: _t->buttonGroupClicked((*reinterpret_cast<std::add_pointer_t<QAbstractButton*>>(_a[1]))); break;
        case 5: _t->deleteItem(); break;
        case 6: _t->pointerGroupClicked(); break;
        case 7: _t->bringToFront(); break;
        case 8: _t->sendToBack(); break;
        case 9: _t->itemInserted((*reinterpret_cast<std::add_pointer_t<DiagramItem*>>(_a[1]))); break;
        case 10: _t->textInserted((*reinterpret_cast<std::add_pointer_t<QGraphicsTextItem*>>(_a[1]))); break;
        case 11: _t->currentFontChanged((*reinterpret_cast<std::add_pointer_t<QFont>>(_a[1]))); break;
        case 12: _t->fontSizeChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->sceneScaleChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->textButtonTriggered(); break;
        case 15: _t->fillButtonTriggered(); break;
        case 16: _t->lineButtonTriggered(); break;
        case 17: _t->handleFontChange(); break;
        case 18: _t->itemSelected((*reinterpret_cast<std::add_pointer_t<QGraphicsItem*>>(_a[1]))); break;
        case 19: _t->about(); break;
        case 20: { bool _r = _t->saveSceneAsImage();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 21: _t->closeEvent((*reinterpret_cast<std::add_pointer_t<QCloseEvent*>>(_a[1]))); break;
        case 22: _t->backgroundChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 23: _t->newScene(); break;
        case 24: _t->sceneymChanged(); break;
        case 25: _t->closeScene((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 26: _t->showContextMenu((*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 27: _t->pasteItemsFromMenu(); break;
        case 28: _t->sceneChanged(); break;
        case 29: _t->openFindReplaceDialog(); break;
        case 30: _t->handleFindText((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 31: _t->handleReplaceText((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 32: _t->handleReplaceAllText((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 33: _t->combination(); break;
        case 34: _t->cancelCombination(); break;
        case 35: _t->savefile(); break;
        case 36: _t->saveSaveFilePath((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 37: _t->changebackground(); break;
        case 38: { bool _r = _t->saveSceneAsImageOrSvg();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 39: { QString _r = _t->loadSaveFilePath();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 40: _t->loadfile(); break;
        case 41: { QList<WriteDiagramItem*> _r = _t->getStructList();
            if (_a[0]) *reinterpret_cast<QList<WriteDiagramItem*>*>(_a[0]) = std::move(_r); }  break;
        case 42: { QList<WriteDiagramPath*> _r = _t->getStructList1();
            if (_a[0]) *reinterpret_cast<QList<WriteDiagramPath*>*>(_a[0]) = std::move(_r); }  break;
        case 43: { QString _r = _t->savefilestack();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 44: _t->loadfilestack((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 45: _t->autoCleanStack(); break;
        case 46: _t->undo(); break;
        case 47: _t->redo(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAbstractButton* >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAbstractButton* >(); break;
            }
            break;
        case 10:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QGraphicsTextItem* >(); break;
            }
            break;
        case 18:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QGraphicsItem* >(); break;
            }
            break;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 48)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 48;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 48)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 48;
    }
    return _id;
}
QT_WARNING_POP
