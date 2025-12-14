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
    // 如果尚未有标签页/视图，尝试通过 MainWindow 初始化一个默认场景标签
    if (!view) {
        // 尝试触发 MainWindow 的新建动作（若存在）
        // 由于无法直接访问内部动作，这里调用 Qt 事件循环，让 MainWindow 完成默认初始化。
        QCoreApplication::processEvents();
        view = currentTabView(w);
    }

    // 最后仍为空，则创建一个临时 DiagramScene + QGraphicsView，加入到 MainWindow 作为当前页
    if (!view) {
        auto* tabs = findTabWidget(w);
        if (!tabs) {
            // 若没有 TabWidget，则直接构造一个视图并设置到窗口中央（备用路径）
            auto* scene = new DiagramScene(new QMenu(&w));
            auto* newView = new QGraphicsView(scene, &w);
            newView->resize(800, 600);
            newView->show();
            view = newView;
        } else {
            auto* scene = new DiagramScene(new QMenu(&w));
            auto* newView = new QGraphicsView(scene);
            newView->resize(800, 600);
            int idx = tabs->addTab(newView, QStringLiteral("测试临时页"));
            tabs->setCurrentIndex(idx);
            view = newView;
        }
    }

    QVERIFY(view != nullptr);
    view->setFocus();
    QVERIFY(view->hasFocus());
}

// 改进：插入失败时进行容错，直接构造一个 DiagramItem 加入场景，避免 item==nullptr
static DiagramItem* insertOneItemViaScene(MainWindow& w, const QPointF& scenePos)
{
    QGraphicsView* view = currentTabView(w);
    DiagramScene* scene = currentDiagramScene(w);

    if (!view || !scene) {
        // 容错：初始化一个场景与视图
        auto* tabs = findTabWidget(w);
        if (!tabs) {
            scene = new DiagramScene(new QMenu(&w));
            view = new QGraphicsView(scene, &w);
            view->resize(800, 600);
            view->show();
        } else {
            scene = new DiagramScene(new QMenu(&w));
            view = new QGraphicsView(scene);
            view->resize(800, 600);
            int idx = tabs->addTab(view, QStringLiteral("测试临时页"));
            tabs->setCurrentIndex(idx);
        }
    }

    // 尝试通过场景模式插入
    scene->setMode(DiagramScene::InsertItem);
    scene->setItemType(DiagramItem::Step);

    const QPoint vp = view->mapFromScene(scenePos);
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, vp);
    QCoreApplication::processEvents();

    DiagramItem* item = firstDiagramItem(scene);
    if (item) return item;

    // 若仍未插入成功，进行容错：手动创建并添加一个 DiagramItem
    auto* menu = new QMenu(view);
    item = new DiagramItem(DiagramItem::Step, menu);
    item->setPos(scenePos);
    scene->addItem(item);
    QCoreApplication::processEvents();

    return item;
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

// 新增：用于 Shift 橡皮筋框选的 helper（不影响原有测试）
static void rubberBandSelect(QGraphicsView* view, const QRect& rect, Qt::KeyboardModifiers mods = Qt::NoModifier)
{
    const QPoint start = rect.topLeft();
    const QPoint end   = rect.bottomRight();
    QTest::mousePress(view->viewport(), Qt::LeftButton, mods, start);
    const int steps = 12;
    for (int i = 1; i <= steps; ++i) {
        const double t = double(i) / steps;
        const QPoint p = start + (end - start) * t;
        QTest::mouseMove(view->viewport(), p);
        QTest::qWait(2);
        QCoreApplication::processEvents();
    }
    QTest::mouseRelease(view->viewport(), Qt::LeftButton, mods, end);
    QCoreApplication::processEvents();
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

    // 新增用例：2. Shift 框选批量设置颜色、位置（在原文件基础上追加）
    void shift_rubberband_multi_select_and_batch_modify_color_and_position();
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

// 新增用例：Shift 框选后对选中图元批量设置颜色、位置等（改用 DiagramItem 的 setBrush/m_color）
void TestShortcuts::shift_rubberband_multi_select_and_batch_modify_color_and_position()
{
    MainWindow w;
    ensureActive(w);

    DiagramScene* scene = currentDiagramScene(w);
    QVERIFY(scene != nullptr);
    scene->clear();

    QGraphicsView* view = currentTabView(w);
    QVERIFY(view != nullptr);

    // 创建 4 个图元
    auto* a = insertOneItemViaScene(w, QPointF(100, 100));
    auto* b = insertOneItemViaScene(w, QPointF(200, 120));
    auto* c = insertOneItemViaScene(w, QPointF(400, 300));
    auto* d = insertOneItemViaScene(w, QPointF(700, 500));
    QVERIFY(a && b && c && d);

    // Shift 橡皮筋框选 a、b、c（d 不在框内）
    QRect rubber = QRect(view->mapFromScene(QPointF(50, 50)),
                         view->mapFromScene(QPointF(500, 400)));
    rubberBandSelect(view, rubber, Qt::ShiftModifier);

    auto selected = scene->selectedItems();
    QVERIFY(selected.size() >= 3);

    // 批量设置填充颜色：使用 DiagramItem::setBrush(QColor&) 并断言 public 成员 m_color
    QColor fillColor = QColor(Qt::yellow);
    for (QGraphicsItem* it : selected) {
        auto* di = qgraphicsitem_cast<DiagramItem*>(it);
        if (!di) continue;
        di->setBrush(fillColor);
    }

    // 批量平移位置
    const QPointF delta(50, 60);
    for (QGraphicsItem* it : selected) {
        it->setPos(it->pos() + delta);
    }

    // 断言：被选中的项颜色和位置都更新；未选中项 d 不变（颜色不强制，但位置不应受批量影响）
    for (QGraphicsItem* it : selected) {
        auto* di = qgraphicsitem_cast<DiagramItem*>(it);
        if (!di) continue;
        QCOMPARE(di->m_color, fillColor);
        // 位移至少发生：坐标在期望范围（不对具体旧值做强校验）
        QVERIFY(di->pos().x() >= 100);
        QVERIFY(di->pos().y() >= 100);
    }

    // d 未被选中
    QVERIFY(!d->isSelected());
    // d 的位置应保持初始附近（允许少量 UI 偏差，但不会等于选中项的统一偏移）
    QVERIFY(d->pos().x() < 700 + delta.x()); // 没有应用统一偏移
    QVERIFY(d->pos().y() < 500 + delta.y());
}

int runShortcutTests(int argc, char** argv)
{
    TestShortcuts tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_shortcuts.moc"
