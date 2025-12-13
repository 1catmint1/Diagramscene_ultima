#include <QtTest/QtTest>
#include <QGraphicsView>
#include <QMenu>
#include <QApplication>

#include "diagramscene.h"
#include "diagramitem.h"
#include "diagramtextitem.h"
#include "arrow.h"
#include "diagrampath.h"

static void ensureActive(QGraphicsView& view)
{
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    view.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&view));
    view.setFocus();
    QVERIFY(view.hasFocus());
    view.viewport()->setMouseTracking(true);
}

static int countType(QGraphicsScene& scene, int type)
{
    int c = 0;
    for (QGraphicsItem* it : scene.items())
        if (it->type() == type) ++c;
    return c;
}

static void dragOnView(QGraphicsView& view, const QPoint& start, const QPoint& end)
{
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QCoreApplication::processEvents();

    const int segments = 12;
    for (int i = 1; i <= segments; ++i) {
        const double t = double(i) / segments;
        const QPoint p = start + (end - start) * t;
        QTest::mouseMove(view.viewport(), p);
        QTest::qWait(2);
        QCoreApplication::processEvents();
    }

    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QCoreApplication::processEvents();
}

// 在 item 的 sceneBoundingRect 内找一个点，使得 scene.items(p).first() == item
// 这正好满足 DiagramScene::mouseReleaseEvent 的 Arrow 创建条件（startItems.first()/endItems.first()）
static QPointF findItemsFirstHitPoint(QGraphicsScene& scene, QGraphicsItem* target)
{
    const QRectF r = target->sceneBoundingRect().adjusted(3, 3, -3, -3);

    auto isFirst = [&](const QPointF& p) {
        const QList<QGraphicsItem*> hits = scene.items(p);
        return !hits.isEmpty() && hits.first() == target;
    };

    // 先试几个典型点：中心 + 四周
    const QList<QPointF> candidates = {
        r.center(),
        QPointF(r.left() + r.width()*0.2, r.center().y()),
        QPointF(r.left() + r.width()*0.8, r.center().y()),
        QPointF(r.center().x(), r.top() + r.height()*0.2),
        QPointF(r.center().x(), r.top() + r.height()*0.8),
    };
    for (const QPointF& p : candidates)
        if (isFirst(p)) return p;

    // 再做更密集扫描
    const int steps = 14;
    for (int iy = 1; iy <= steps; ++iy) {
        for (int ix = 1; ix <= steps; ++ix) {
            QPointF p(r.left() + r.width() * ix / (steps + 1),
                      r.top() + r.height() * iy / (steps + 1));
            if (isFirst(p)) return p;
        }
    }

    // 实在不行就返回中心（会导致测试失败并输出调试信息）
    return r.center();
}

class TestSceneManagement : public QObject
{
    Q_OBJECT
private slots:
    void mode_insertItem_createsDiagramItem();
    void mode_insertText_createsTextItem();
    void mode_insertLine_createsArrow_between_two_items();
    void mode_insertPath_createsDiagramPath_between_two_items();
    void mode_moveItem_drag_shouldMoveItem();
    void keyboard_shortcut_rotate_selected_item();
};

void TestSceneManagement::mode_insertItem_createsDiagramItem()
{
    QMenu menu;
    DiagramScene scene(&menu);
    scene.setSceneRect(0, 0, 800, 600);

    QGraphicsView view(&scene);
    view.resize(800, 600);
    ensureActive(view);

    scene.setMode(DiagramScene::InsertItem);
    scene.setItemType(DiagramItem::Step);

    const QPoint p = view.mapFromScene(QPointF(120, 120));
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, p);
    QCoreApplication::processEvents();

    QVERIFY(countType(scene, DiagramItem::Type) >= 1);
}

void TestSceneManagement::mode_insertText_createsTextItem()
{
    QMenu menu;
    DiagramScene scene(&menu);
    scene.setSceneRect(0, 0, 800, 600);

    QGraphicsView view(&scene);
    view.resize(800, 600);
    ensureActive(view);

    scene.setMode(DiagramScene::InsertText);

    const QPoint p = view.mapFromScene(QPointF(200, 200));
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, p);
    QCoreApplication::processEvents();

    QVERIFY(countType(scene, DiagramTextItem::Type) >= 1);
}

void TestSceneManagement::mode_insertLine_createsArrow_between_two_items()
{
    QMenu menu;
    DiagramScene scene(&menu);
    scene.setSceneRect(0, 0, 800, 600);

    auto* startItem = new DiagramItem(DiagramItem::Step, &menu);
    auto* endItem = new DiagramItem(DiagramItem::Step, &menu);
    startItem->setPos(120, 140);
    endItem->setPos(520, 360);
    scene.addItem(startItem);
    scene.addItem(endItem);
    QCoreApplication::processEvents();

    QGraphicsView view(&scene);
    view.resize(800, 600);
    ensureActive(view);

    scene.setMode(DiagramScene::InsertLine);

    const int before = countType(scene, Arrow::Type);

    const QPointF startScene = findItemsFirstHitPoint(scene, startItem);
    const QPointF endScene   = findItemsFirstHitPoint(scene, endItem);

    const QPoint start = view.mapFromScene(startScene);
    const QPoint end   = view.mapFromScene(endScene);

    dragOnView(view, start, end);

    const int after = countType(scene, Arrow::Type);

    QVERIFY2(after >= before + 1,
             qPrintable(QString("Arrow not created. before=%1 after=%2 startScene=(%3,%4) endScene=(%5,%6)")
                            .arg(before).arg(after)
                            .arg(startScene.x()).arg(startScene.y())
                            .arg(endScene.x()).arg(endScene.y())));
}

void TestSceneManagement::mode_insertPath_createsDiagramPath_between_two_items()
{
    QMenu menu;
    DiagramScene scene(&menu);
    scene.setSceneRect(0, 0, 800, 600);

    auto* a = new DiagramItem(DiagramItem::Step, &menu);
    auto* b = new DiagramItem(DiagramItem::Step, &menu);
    a->setPos(150, 150);
    b->setPos(450, 280);
    scene.addItem(a);
    scene.addItem(b);

    QGraphicsView view(&scene);
    view.resize(800, 600);
    ensureActive(view);

    scene.setMode(DiagramScene::InsertPath);

    QPointF aLinkScene = a->mapToScene(a->linkWhere()[DiagramItem::TF_Right].center());
    QPointF bLinkScene = b->mapToScene(b->linkWhere()[DiagramItem::TF_Left].center());

    const QPoint start = view.mapFromScene(aLinkScene);
    const QPoint end   = view.mapFromScene(bLinkScene);

    dragOnView(view, start, end);

    QVERIFY(countType(scene, DiagramPath::Type) >= 1);
}

void TestSceneManagement::mode_moveItem_drag_shouldMoveItem()
{
    QMenu menu;
    DiagramScene scene(&menu);
    scene.setSceneRect(0, 0, 800, 600);

    auto* item = new DiagramItem(DiagramItem::Step, &menu);
    item->setPos(200, 150);
    scene.addItem(item);
    QCoreApplication::processEvents();

    QGraphicsView view(&scene);
    view.resize(800, 600);
    ensureActive(view);

    scene.setMode(DiagramScene::MoveItem);

    const QPointF pressScene = findItemsFirstHitPoint(scene, item);
    const QPoint press = view.mapFromScene(pressScene);
    const QPoint release = press + QPoint(220, 160);

    const QPointF oldPos = item->pos();
    dragOnView(view, press, release);

    const QPointF newPos = item->pos();
    const qreal dx = qAbs(newPos.x() - oldPos.x());
    const qreal dy = qAbs(newPos.y() - oldPos.y());

    QVERIFY2(dx > 2 || dy > 2,
             qPrintable(QString("MoveItem did not move. old=(%1,%2) new=(%3,%4) pressScene=(%5,%6)")
                            .arg(oldPos.x()).arg(oldPos.y())
                            .arg(newPos.x()).arg(newPos.y())
                            .arg(pressScene.x()).arg(pressScene.y())));
}

void TestSceneManagement::keyboard_shortcut_rotate_selected_item()
{
    QMenu menu;
    DiagramScene scene(&menu);
    scene.setSceneRect(0, 0, 800, 600);

    QGraphicsView view(&scene);
    view.resize(800, 600);
    ensureActive(view);

    auto* item = new DiagramItem(DiagramItem::Step, &menu);
    item->setPos(100, 100);
    scene.addItem(item);
    item->setSelected(true);
    QCOMPARE(scene.selectedItems().size(), 1);

    const qreal a0 = item->rotationAngle();

    QTest::keyClick(&view, Qt::Key_R);
    QCOMPARE(item->rotationAngle(), a0 + 5);

    QTest::keyClick(&view, Qt::Key_L);
    QCOMPARE(item->rotationAngle(), a0);
}

int runSceneManagementTests(int argc, char** argv)
{
    TestSceneManagement tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_scene_management.moc"
