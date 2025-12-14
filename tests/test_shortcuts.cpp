#include <QtTest/QtTest>
#include <QAction>
#include <QKeySequence>
#include <QTabWidget>
#include <QGraphicsView>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTemporaryDir>
#include <QDir>

#include "../mainwindow.h"
#include "../diagramitem.h"
#include "../diagramscene.h"

// ---------------- helpers ----------------
// 注意：不要在“返回非 void”的 helper 里用 QVERIFY/QCOMPARE（失败时会 return; 导致编译错误）

static QTabWidget* findTabWidget(MainWindow& w)
{
    return w.findChild<QTabWidget*>();
}

static QGraphicsView* currentTabView(MainWindow& w)
{
    QTabWidget* tabs = findTabWidget(w);
    if (!tabs) return nullptr;
    return qobject_cast<QGraphicsView*>(tabs->currentWidget());
}

static DiagramScene* currentDiagramScene(MainWindow& w)
{
    auto* view = currentTabView(w);
    if (!view) return nullptr;
    return qobject_cast<DiagramScene*>(view->scene());
}

static int countDiagramItems(QGraphicsScene* scene)
{
    int c = 0;
    for (QGraphicsItem* it : scene->items())
        if (it->type() == DiagramItem::Type) ++c;
    return c;
}

static DiagramItem* firstDiagramItem(QGraphicsScene* scene)
{
    for (QGraphicsItem* it : scene->items())
        if (it->type() == DiagramItem::Type)
            return qgraphicsitem_cast<DiagramItem*>(it);
    return nullptr;
}

static void ensureActive(MainWindow& w)
{
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));
    w.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&w));

    auto* view = currentTabView(w);
    QVERIFY(view != nullptr);
    view->setFocus();
    QVERIFY(view->hasFocus());
}

static DiagramItem* insertOneItemViaScene(MainWindow& w, const QPointF& scenePos)
{
    QGraphicsView* view = currentTabView(w);
    if (!view) return nullptr;

    DiagramScene* scene = currentDiagramScene(w);
    if (!scene) return nullptr;

    scene->setMode(DiagramScene::InsertItem);
    scene->setItemType(DiagramItem::Step);

    const QPoint vp = view->mapFromScene(scenePos);
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, vp);
    QCoreApplication::processEvents();

    return firstDiagramItem(scene);
}

static bool hasActionWithShortcut(MainWindow& w, const QKeySequence& seq)
{
    const auto actions = w.findChildren<QAction*>();
    for (QAction* a : actions) {
        if (!a) continue;
        if (a->shortcut() == seq) return true;
        if (a->shortcuts().contains(seq)) return true;
    }
    return false;
}

static int countActionsWithShortcut(MainWindow& w, const QKeySequence& seq)
{
    int n = 0;
    const auto actions = w.findChildren<QAction*>();
    for (QAction* a : actions) {
        if (!a) continue;
        if (a->shortcut() == seq) ++n;
        else if (a->shortcuts().contains(seq)) ++n;
    }
    return n;
}

// ---------------- tests ----------------

class TestShortcuts : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase()
    {
        QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, true);
        QApplication::clipboard()->clear();
    }

    void cleanupTestCase()
    {
        QApplication::clipboard()->clear();
    }

    void shortcut_mapping_exists_and_no_conflict();

    void ctrlC_copies_selected_item_to_clipboard();
    void ctrlX_cuts_selected_item();
    void ctrlV_pastes_item();
    void delete_deletes_selected_item();

    void ctrlR_ctrlL_rotate_selected_item();

    void ctrlZ_undo_and_ctrlY_redo_actual_scene_change();
};

void TestShortcuts::shortcut_mapping_exists_and_no_conflict()
{
    MainWindow w;
    ensureActive(w);

    struct Entry { QKeySequence seq; const char* name; };
    const Entry entries[] = {
                             { QKeySequence(Qt::CTRL | Qt::Key_C), "Ctrl+C (Copy)" },
                             { QKeySequence(Qt::CTRL | Qt::Key_V), "Ctrl+V (Paste)" },
                             { QKeySequence(Qt::CTRL | Qt::Key_X), "Ctrl+X (Cut)" },
                             { QKeySequence(Qt::Key_Delete),       "Delete" },
                             { QKeySequence::Undo,                 "Undo (Ctrl+Z typically)" },
                             { QKeySequence::Redo,                 "Redo (Ctrl+Y / Ctrl+Shift+Z typically)" },
                             };

    for (const auto& e : entries) {
        QVERIFY2(hasActionWithShortcut(w, e.seq),
                 qPrintable(QString("Missing shortcut mapping for %1").arg(e.name)));

        const int cnt = countActionsWithShortcut(w, e.seq);
        QVERIFY2(cnt <= 1,
                 qPrintable(QString("Shortcut conflict: %1 bound to %2 actions").arg(e.name).arg(cnt)));
    }
}

void TestShortcuts::ctrlC_copies_selected_item_to_clipboard()
{
    MainWindow w;
    ensureActive(w);

    DiagramScene* scene = currentDiagramScene(w);
    QVERIFY(scene != nullptr);
    scene->clear();

    DiagramItem* item = insertOneItemViaScene(w, QPointF(200, 200));
    QVERIFY(item != nullptr);
    item->setSelected(true);

    QTest::keyClick(&w, Qt::Key_C, Qt::ControlModifier);
    QCoreApplication::processEvents();

    const QMimeData* md = QApplication::clipboard()->mimeData();
    QVERIFY(md != nullptr);
    QVERIFY(md->hasFormat("application/x-diagramscene-item-type"));
}

void TestShortcuts::ctrlX_cuts_selected_item()
{
    MainWindow w;
    ensureActive(w);

    DiagramScene* scene = currentDiagramScene(w);
    QVERIFY(scene != nullptr);
    scene->clear();

    DiagramItem* item = insertOneItemViaScene(w, QPointF(220, 220));
    QVERIFY(item != nullptr);
    item->setSelected(true);
    QCOMPARE(countDiagramItems(scene), 1);

    QTest::keyClick(&w, Qt::Key_X, Qt::ControlModifier);
    QCoreApplication::processEvents();

    QCOMPARE(countDiagramItems(scene), 0);

    const QMimeData* md = QApplication::clipboard()->mimeData();
    QVERIFY(md != nullptr);
    QVERIFY(md->hasFormat("application/x-diagramscene-item-type"));
}

void TestShortcuts::ctrlV_pastes_item()
{
    MainWindow w;
    ensureActive(w);

    DiagramScene* scene = currentDiagramScene(w);
    QVERIFY(scene != nullptr);
    scene->clear();

    DiagramItem* item = insertOneItemViaScene(w, QPointF(200, 200));
    QVERIFY(item != nullptr);
    item->setSelected(true);

    QTest::keyClick(&w, Qt::Key_C, Qt::ControlModifier);
    QCoreApplication::processEvents();

    scene->clear();
    QCOMPARE(countDiagramItems(scene), 0);

    QGraphicsView* view = currentTabView(w);
    QVERIFY(view != nullptr);
    QTest::mouseMove(view->viewport(), view->viewport()->rect().center());
    QCoreApplication::processEvents();

    QTest::keyClick(&w, Qt::Key_V, Qt::ControlModifier);
    QCoreApplication::processEvents();

    QVERIFY2(countDiagramItems(scene) >= 1, "Ctrl+V did not paste DiagramItem");
}

void TestShortcuts::delete_deletes_selected_item()
{
    MainWindow w;
    ensureActive(w);

    DiagramScene* scene = currentDiagramScene(w);
    QVERIFY(scene != nullptr);
    scene->clear();

    DiagramItem* item = insertOneItemViaScene(w, QPointF(240, 240));
    QVERIFY(item != nullptr);
    item->setSelected(true);
    QCOMPARE(countDiagramItems(scene), 1);

    QTest::keyClick(&w, Qt::Key_Delete);
    QCoreApplication::processEvents();

    QCOMPARE(countDiagramItems(scene), 0);
}

void TestShortcuts::ctrlR_ctrlL_rotate_selected_item()
{
    MainWindow w;
    ensureActive(w);

    DiagramScene* scene = currentDiagramScene(w);
    QVERIFY(scene != nullptr);
    scene->clear();

    QGraphicsView* view = currentTabView(w);
    QVERIFY(view != nullptr);

    DiagramItem* item = insertOneItemViaScene(w, QPointF(250, 250));
    QVERIFY(item != nullptr);

    // 点击确保选中
    QPoint vp = view->mapFromScene(item->sceneBoundingRect().center());
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, vp);
    QCoreApplication::processEvents();
    QVERIFY(item->isSelected());

    view->viewport()->setFocus();
    QVERIFY(view->viewport()->hasFocus());

    const qreal a0 = item->rotationAngle();

    // 你的需求写的是 Ctrl+R / Ctrl+L
    QTest::keyClick(view->viewport(), Qt::Key_R, Qt::ControlModifier);
    QCoreApplication::processEvents();
    QVERIFY2(qAbs(item->rotationAngle() - (a0 + 5)) < 1e-6,
             qPrintable(QString("Ctrl+R rotate failed: actual=%1 expected=%2")
                            .arg(item->rotationAngle()).arg(a0 + 5)));

    QTest::keyClick(view->viewport(), Qt::Key_L, Qt::ControlModifier);
    QCoreApplication::processEvents();
    QVERIFY2(qAbs(item->rotationAngle() - a0) < 1e-6,
             qPrintable(QString("Ctrl+L rotate failed: actual=%1 expected=%2")
                            .arg(item->rotationAngle()).arg(a0)));
}

void TestShortcuts::ctrlZ_undo_and_ctrlY_redo_actual_scene_change()
{
    // Undo/Redo 依赖 stacks/*.fcproj 文件。为了不污染项目目录，把工作目录切到临时目录。
    QTemporaryDir tmp;
    QVERIFY2(tmp.isValid(), "Failed to create temporary directory for undo/redo test.");
    const QString oldCwd = QDir::currentPath();
    QVERIFY(QDir::setCurrent(tmp.path()));

    MainWindow w;
    ensureActive(w);

    // 注意：undo/redo 会调用 loadfilestack，内部 scene->clear 并重建 items，
    // 所以不能长期缓存 scene 指针；每次都重新 currentDiagramScene(w)
    DiagramScene* scene0 = currentDiagramScene(w);
    QVERIFY(scene0 != nullptr);
    scene0->clear();

    // 插入第 1 个图元（会触发 itemInserted → savefilestack() 连接，自动产生快照）
    DiagramItem* a = insertOneItemViaScene(w, QPointF(200, 200));
    QVERIFY(a != nullptr);
    QCoreApplication::processEvents();

    // 插入第 2 个图元（再次自动快照）
    DiagramItem* b = insertOneItemViaScene(w, QPointF(320, 260));
    QVERIFY(b != nullptr);
    QCoreApplication::processEvents();

    DiagramScene* sceneBefore = currentDiagramScene(w);
    QVERIFY(sceneBefore != nullptr);
    const int beforeUndo = countDiagramItems(sceneBefore);
    QVERIFY2(beforeUndo >= 2, qPrintable(QString("Expected >=2 items before undo, got %1").arg(beforeUndo)));

    // Ctrl+Z：回退到上一个快照（通常少一个图元）
    QTest::keyClick(&w, Qt::Key_Z, Qt::ControlModifier);
    QCoreApplication::processEvents();

    auto afterUndoCount = [&]() -> int {
        DiagramScene* s = currentDiagramScene(w);
        if (!s) return -1;
        return countDiagramItems(s);
    };

    QTRY_VERIFY2(afterUndoCount() >= 0 && afterUndoCount() < beforeUndo,
                 qPrintable(QString("Undo did not reduce item count: before=%1 after=%2")
                                .arg(beforeUndo).arg(afterUndoCount())));

    const int afterUndo = afterUndoCount();

    // Ctrl+Y：恢复撤销的那一步（重做），数量应增加
    QTest::keyClick(&w, Qt::Key_Y, Qt::ControlModifier);
    QCoreApplication::processEvents();

    QTRY_VERIFY2(afterUndoCount() > afterUndo,
                 qPrintable(QString("Redo did not increase item count: afterUndo=%1 afterRedo=%2")
                                .arg(afterUndo).arg(afterUndoCount())));

    // 恢复工作目录
    QVERIFY(QDir::setCurrent(oldCwd));
}

int runShortcutTests(int argc, char** argv)
{
    TestShortcuts tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_shortcuts.moc"
