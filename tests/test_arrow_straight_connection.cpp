#include <QtTest/QtTest>
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>
#include <QMenu>
#include <QLineF>
#include <QtMath>

#include "../diagramitem.h"
#include "../arrow.h"

// ===================== 工具函数 =====================

static void forceOneRender(QGraphicsScene &scene, const QSize &imgSize = QSize(900, 700))
{
    QImage img(imgSize, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing, false);
    scene.render(&p);
    p.end();
}

static qreal pointToSegmentDistance(const QPointF &p, const QPointF &a, const QPointF &b)
{
    const QPointF ab = b - a;
    const QPointF ap = p - a;
    const qreal ab2 = QPointF::dotProduct(ab, ab);
    if (ab2 <= 1e-12) return QLineF(p, a).length();

    qreal t = QPointF::dotProduct(ap, ab) / ab2;
    t = qMax<qreal>(0.0, qMin<qreal>(1.0, t));
    const QPointF proj = a + t * ab;
    return QLineF(p, proj).length();
}

static qreal distanceToPolygonBoundaryScene(const QPointF &ptScene, const QPolygonF &polyScene)
{
    if (polyScene.size() < 2) return 1e18;

    qreal best = 1e18;
    for (int i = 0; i < polyScene.size(); ++i) {
        const QPointF a = polyScene[i];
        const QPointF b = polyScene[(i + 1) % polyScene.size()];
        best = qMin(best, pointToSegmentDistance(ptScene, a, b));
    }
    return best;
}

static qreal angleDeg(const QPointF &v1, const QPointF &v2)
{
    const qreal n1 = qSqrt(QPointF::dotProduct(v1, v1));
    const qreal n2 = qSqrt(QPointF::dotProduct(v2, v2));
    if (n1 <= 1e-12 || n2 <= 1e-12) return 0.0;

    qreal c = QPointF::dotProduct(v1, v2) / (n1 * n2);
    c = qMax<qreal>(-1.0, qMin<qreal>(1.0, c));
    return qRadiansToDegrees(qAcos(c));
}

static QPolygonF endPolygonScene(DiagramItem *endItem)
{
    const QPolygonF polyLocal = endItem->polygon();
    return endItem->mapToScene(polyLocal);
}

// ✅ 更鲁棒的断言：
// - head 在边界附近（dist <= tol） => 通过
// - 或 head 在 polygon 内部且离边界不远（dist <= innerTol）=> 通过
static void assertHeadNearEndBoundaryOrInside(DiagramItem *endItem,
                                              const QPointF &headScene,
                                              qreal tol = 6.0,
                                              qreal innerTol = 8.0)
{
    const QPolygonF polyScene = endPolygonScene(endItem);
    QVERIFY2(polyScene.size() >= 2,
             "endItem->polygon() 为空：请确认已 render 触发过 DiagramItem::paint() 来填充 polygon");

    const qreal d = distanceToPolygonBoundaryScene(headScene, polyScene);

    if (d <= tol)
        return;

    // 如果点落在 polygon 内部（多半是交点算在边界内侧几个像素），允许更宽松一点
    const bool inside = polyScene.containsPoint(headScene, Qt::OddEvenFill);
    QVERIFY2(inside && d <= innerTol,
             qPrintable(QString("箭头头部不在 endItem 边界附近/内部可接受范围: dist=%1 tol=%2 innerTol=%3 inside=%4")
                            .arg(d).arg(tol).arg(innerTol).arg(inside)));
}

static void assertTailAtStartCenter(DiagramItem *startItem,
                                    const QPointF &tailScene,
                                    qreal tol = 1e-6)
{
    const QPointF startCenter = startItem->pos();
    const qreal d = QLineF(tailScene, startCenter).length();

    QVERIFY2(d <= tol,
             qPrintable(QString("箭头尾部不在 startItem 中心: dist=%1 tol=%2  tail=(%3,%4) start=(%5,%6)")
                            .arg(d).arg(tol)
                            .arg(tailScene.x()).arg(tailScene.y())
                            .arg(startCenter.x()).arg(startCenter.y())));
}

static void assertArrowDirectionCorrect(DiagramItem *startItem,
                                        DiagramItem *endItem,
                                        const QPointF &tailScene,
                                        const QPointF &headScene,
                                        qreal tolDeg = 12.0)
{
    const QPointF vArrow  = headScene - tailScene;
    const QPointF vCenter = endItem->pos() - startItem->pos();

    const qreal dot = QPointF::dotProduct(vArrow, vCenter);
    QVERIFY2(dot > 0, "Arrow 方向错误：箭头指向与 start->end 方向反向（dot<=0）");

    const qreal ang = angleDeg(vArrow, vCenter);
    QVERIFY2(ang <= tolDeg,
             qPrintable(QString("Arrow 指向偏差过大：angle=%1 deg (tol=%2 deg)")
                            .arg(ang).arg(tolDeg)));
}

static void getArrowHeadTailScene(Arrow *arrow, QPointF &headScene, QPointF &tailScene)
{
    const QLineF Llocal = arrow->line();
    headScene = arrow->mapToScene(Llocal.p1());
    tailScene = arrow->mapToScene(Llocal.p2());
}

// ===================== 测试类 =====================

class TestArrowStraightConnection : public QObject
{
    Q_OBJECT
private slots:
    void straightConnection_basic_and_stable();
    void straightConnection_afterMove();
};

void TestArrowStraightConnection::straightConnection_basic_and_stable()
{
    QGraphicsScene scene;
    QMenu menu;

    auto *start = new DiagramItem(DiagramItem::Step, &menu, nullptr);
    auto *end   = new DiagramItem(DiagramItem::Step, &menu, nullptr);

    start->setFixedSize(QSizeF(200, 100));
    end->setFixedSize(QSizeF(200, 100));

    start->setPos(100, 100);
    end->setPos(450, 120);

    scene.addItem(start);
    scene.addItem(end);

    auto *arrow = new Arrow(start, end);
    scene.addItem(arrow);

    forceOneRender(scene);

    QPointF head0, tail0;
    getArrowHeadTailScene(arrow, head0, tail0);

    // 连接点：头在 end 边界附近/或内部可接受范围；尾在 start 中心
    assertHeadNearEndBoundaryOrInside(end, head0, 6.0, 8.0);
    assertTailAtStartCenter(start, tail0, 1e-6);

    // 指向正确
    assertArrowDirectionCorrect(start, end, tail0, head0, 12.0);

    // 稳定性：重复 update + render 不漂移
    for (int i = 0; i < 25; ++i) {
        arrow->updatePosition();
        forceOneRender(scene);

        QPointF headI, tailI;
        getArrowHeadTailScene(arrow, headI, tailI);

        QVERIFY2(QLineF(headI, head0).length() < 1e-6, "重复 update/render 后 head 漂移");
        QVERIFY2(QLineF(tailI, tail0).length() < 1e-6, "重复 update/render 后 tail 漂移");
    }

    delete arrow;
    delete start;
    delete end;
}

void TestArrowStraightConnection::straightConnection_afterMove()
{
    QGraphicsScene scene;
    QMenu menu;

    auto *start = new DiagramItem(DiagramItem::Step, &menu, nullptr);
    auto *end   = new DiagramItem(DiagramItem::Step, &menu, nullptr);

    start->setFixedSize(QSizeF(200, 100));
    end->setFixedSize(QSizeF(200, 100));

    start->setPos(200, 200);
    end->setPos(600, 200);

    scene.addItem(start);
    scene.addItem(end);

    auto *arrow = new Arrow(start, end);
    scene.addItem(arrow);

    forceOneRender(scene);

    QPointF head1, tail1;
    getArrowHeadTailScene(arrow, head1, tail1);

    assertHeadNearEndBoundaryOrInside(end, head1, 6.0, 8.0);
    assertTailAtStartCenter(start, tail1, 1e-6);
    assertArrowDirectionCorrect(start, end, tail1, head1, 10.0);

    // 移动 endItem
    end->setPos(520, 420);

    forceOneRender(scene);

    QPointF head2, tail2;
    getArrowHeadTailScene(arrow, head2, tail2);

    QVERIFY2(QLineF(head2, head1).length() > 1.0, "移动 endItem 后 arrow head 未更新（变化过小）");

    assertTailAtStartCenter(start, tail2, 1e-6);
    assertHeadNearEndBoundaryOrInside(end, head2, 6.0, 8.0);
    assertArrowDirectionCorrect(start, end, tail2, head2, 12.0);

    delete arrow;
    delete start;
    delete end;
}

// QTEST_MAIN(TestArrowStraightConnection)
#include "test_arrow_straight_connection.moc"
