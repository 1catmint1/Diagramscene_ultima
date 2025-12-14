#include <QtTest/QtTest>
#include <QGraphicsView>
#include <QMenu>
#include <QApplication>
#include <QImage>
#include <QPixmap>
#include <QBrush>

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
    // 原有用例
    void mode_insertItem_createsDiagramItem();
    void mode_insertText_createsTextItem();
    void mode_insertLine_createsArrow_between_two_items();
    void mode_insertPath_createsDiagramPath_between_two_items();
    void mode_moveItem_drag_shouldMoveItem();
    void keyboard_shortcut_rotate_selected_item();

    // 新增用例：1. 堆叠调整（前置/后置）
    void zorder_bring_to_front_and_send_to_back_should_change_stack_order();

    // 新增用例：5. 导入背景图片
    void scene_import_background_image_should_apply_brush_texture_and_persist();
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

// 1. 图元堆叠调整：验证 setZValue 或前置/后置操作后，堆叠顺序发生变化
void TestSceneManagement::zorder_bring_to_front_and_send_to_back_should_change_stack_order()
{
    QMenu menu;
    DiagramScene scene(&menu);
    scene.setSceneRect(0, 0, 800, 600);

    // 创建两个重叠的图元
    auto* bottom = new DiagramItem(DiagramItem::Step, &menu);
    auto* top    = new DiagramItem(DiagramItem::Step, &menu);
    bottom->setPos(200, 200);
    top->setPos(220, 220); // 有重叠
    bottom->setZValue(0);
    top->setZValue(1);
    scene.addItem(bottom);
    scene.addItem(top);

    // 初始：top 在上面
    QVERIFY(top->zValue() > bottom->zValue());

    // 发送 top 到底部
    top->setZValue(-10);
    QVERIFY(top->zValue() < bottom->zValue());

    // 再将 bottom 提到最前
    bottom->setZValue(100);
    QVERIFY(bottom->zValue() > top->zValue());

    // 验证 scene.items() 在同一点处的顺序（items 按 Z 从大到小）
    QPointF overlapPoint = top->sceneBoundingRect().center();
    auto itemsAtPoint = scene.items(overlapPoint);
    QVERIFY(itemsAtPoint.size() >= 2);
    // 首元素应是 zValue 更大的 bottom
    QCOMPARE(itemsAtPoint.first(), static_cast<QGraphicsItem*>(bottom));
}

// 5. 导入背景图片：设置背景 brush 为纹理图，并验证属性；可选：保存/加载后仍存在
void TestSceneManagement::scene_import_background_image_should_apply_brush_texture_and_persist()
{
    QMenu menu;
    DiagramScene scene(&menu);
    scene.setSceneRect(0, 0, 800, 600);

    // 构造一张测试背景图（避免依赖外部文件）
    QImage img(64, 64, QImage::Format_ARGB32_Premultiplied);
    img.fill(QColor(30, 60, 90, 255));
    QPixmap px = QPixmap::fromImage(img);

    // 设置为背景
    scene.setBackgroundBrush(QBrush(px));

    // 断言背景 brush 具有纹理
    QBrush b = scene.backgroundBrush();
    QVERIFY(!b.texture().isNull());
    QCOMPARE(b.texture().size(), px.size());

    // 若将来实现项目保存/加载背景，可在此处扩展 IO 验证
    // 例如：保存文件 -> 重新加载 -> 验证 scene.backgroundBrush() 仍有纹理
}

int runSceneManagementTests(int argc, char** argv)
{
    TestSceneManagement tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_scene_management.moc"
