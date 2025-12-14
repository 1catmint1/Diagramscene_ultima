#include <QtTest/QtTest>
#include <QAction>
#include <QKeySequence>
#include <QTabWidget>
#include <QGraphicsView>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

#include "../mainwindow.h"
#include "../diagramitem.h"
#include "../diagramscene.h"

// ---------------- helpers (NO QVERIFY/QCOMPARE in non-void functions) ----------------

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

// 用 InsertItem 模式在 scenePos 插入一个 DiagramItem
// 注意：这里不能用 QVERIFY，否则失败会 return; 导致编译错误（本函数返回 DiagramItem*）
static DiagramItem* insertOneItemViaSceneOrNull(MainWindow& w, const QPointF& scenePos)
{
    auto* view = currentTabView(w);
    if (!view) return nullptr;

    auto* scene = currentDiagramScene(w);
    if (!scene) return nullptr;

    scene->setMode(DiagramScene::InsertItem);
    scene->setItemType(DiagramItem::Step);

    const QPoint vp = view->mapFromScene(scenePos);
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, vp);
    QCoreApplication::processEvents();

    return firstDiagramItem(scene);
}

// 在 menuBar()/actions() 里找是否存在给定快捷键的 QAction
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

class TestShortcutsNoSnapshot : public QObject
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
    void ctrlC_ctrlX_ctrlV_delete_functional();
    void key_R_L_rotate_selected_item();
};

void TestShortcutsNoSnapshot::shortcut_mapping_exists_and_no_conflict()
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

void TestShortcutsNoSnapshot::ctrlC_ctrlX_ctrlV_delete_functional()
{
    MainWindow w;
    ensureActive(w);

    auto* scene = currentDiagramScene(w);
    QVERIFY(scene != nullptr);
    scene->clear();

    DiagramItem* item = insertOneItemViaSceneOrNull(w, QPointF(200, 200));
    QVERIFY2(item != nullptr, "Failed to insert DiagramItem via scene (insertOneItemViaSceneOrNull returned null).");

    item->setSelected(true);
    QCOMPARE(countDiagramItems(scene), 1);

    // Ctrl+C
    QTest::keyClick(&w, Qt::Key_C, Qt::ControlModifier);
    QCoreApplication::processEvents();

    const QMimeData* md = QApplication::clipboard()->mimeData();
    QVERIFY(md != nullptr);
    QVERIFY(md->hasFormat("application/x-diagramscene-item-type"));

    // Ctrl+X
    QTest::keyClick(&w, Qt::Key_X, Qt::ControlModifier);
    QCoreApplication::processEvents();
    QCOMPARE(countDiagramItems(scene), 0);

    md = QApplication::clipboard()->mimeData();
    QVERIFY(md != nullptr);
    QVERIFY(md->hasFormat("application/x-diagramscene-item-type"));

    // Ctrl+V：粘贴位置取决于鼠标位置，所以移到 view 中心
    auto* view = currentTabView(w);
    QVERIFY(view != nullptr);
    QTest::mouseMove(view->viewport(), view->viewport()->rect().center());
    QCoreApplication::processEvents();

    QTest::keyClick(&w, Qt::Key_V, Qt::ControlModifier);
    QCoreApplication::processEvents();
    QVERIFY2(countDiagramItems(scene) >= 1, "Ctrl+V did not paste DiagramItem");

    // Delete：删除选中 item
    DiagramItem* pasted = firstDiagramItem(scene);
    QVERIFY(pasted != nullptr);
    pasted->setSelected(true);

    QTest::keyClick(&w, Qt::Key_Delete);
    QCoreApplication::processEvents();
    QCOMPARE(countDiagramItems(scene), 0);
}

void TestShortcutsNoSnapshot::key_R_L_rotate_selected_item()
{
    MainWindow w;
    ensureActive(w);

    auto* scene = currentDiagramScene(w);
    QVERIFY(scene != nullptr);
    scene->clear();

    DiagramItem* item = insertOneItemViaSceneOrNull(w, QPointF(250, 250));
    QVERIFY2(item != nullptr, "Failed to insert DiagramItem for rotate test.");
    item->setSelected(true);

    const qreal a0 = item->rotationAngle();

    QTest::keyClick(&w, Qt::Key_R);
    QCOMPARE(item->rotationAngle(), a0 + 5);

    QTest::keyClick(&w, Qt::Key_L);
    QCOMPARE(item->rotationAngle(), a0);
}

int runShortcutTests(int argc, char** argv)
{
    TestShortcutsNoSnapshot tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_shortcuts.moc"
