#include <QtTest/QtTest>
#include <QGraphicsScene>
#include <QMenu>
#include <QPainterPath>
#include <QLineF>
#include <QSet>
#include <QtMath>

#include "../diagramitem.h"
#include "../diagrampath.h"

// ----------------- 工具函数 -----------------
static qreal dist(const QPointF& a, const QPointF& b) {
    return QLineF(a, b).length();
}

static bool containsPointFuzzy(const QPainterPath& path, const QPointF& p, qreal tol = 1.0) {
    for (int i = 0; i < path.elementCount(); ++i) {
        const auto e = path.elementAt(i);
        const QPointF q(e.x, e.y);
        if (dist(p, q) <= tol) return true;
    }
    return false;
}

static int quadLikeImpl(const QPointF& startPoint, const QPointF& endPoint) {
    if (startPoint.x() >= endPoint.x() && startPoint.y() >= endPoint.y()) return 4;
    if (startPoint.x() <= endPoint.x() && startPoint.y() >= endPoint.y()) return 1;
    if (startPoint.x() >= endPoint.x() && startPoint.y() <= endPoint.y()) return 3;
    if (startPoint.x() <= endPoint.x() && startPoint.y() <= endPoint.y()) return 2;
    return 0;
}

// 复刻 diagrampath.cpp 的 drawZig() 分支：根据 m_state 得到期望折点列表
static QVector<QPointF> expectedZigPoints(const QPointF& s, const QPointF& e, int m_state) {
    const QPointF mid((s.x() + e.x())/2.0, (s.y() + e.y())/2.0);

    // 与源码 switch-case 保持一致
    static const QSet<int> CASE_ENDX_STARTY = {
        882,883,811,812,813,822,823,824,182,111,112,141,411,412,414,441,444,421,423,424,283,244,223,224
    };
    static const QSet<int> CASE_STARTX_ENDY = {
        881,884,814,821,181,183,184,113,114,142,143,144,413,442,443,422,281,282,284,241,242,243,221,222
    };
    static const QSet<int> CASE_MIDX_TWO_VERTICAL = {
        842,843,121,122,481,484,213,214
    };
    static const QSet<int> CASE_MIDY_TWO_HORIZONTAL = {
        841,844,123,124,482,483,211,212
    };

    QVector<QPointF> pts;

    if (CASE_ENDX_STARTY.contains(m_state)) {
        pts.push_back(QPointF(e.x(), s.y()));
    } else if (CASE_STARTX_ENDY.contains(m_state)) {
        pts.push_back(QPointF(s.x(), e.y()));
    } else if (CASE_MIDX_TWO_VERTICAL.contains(m_state)) {
        pts.push_back(QPointF(mid.x(), s.y()));
        pts.push_back(QPointF(mid.x(), e.y()));
    } else if (CASE_MIDY_TWO_HORIZONTAL.contains(m_state)) {
        pts.push_back(QPointF(s.x(), mid.y()));
        pts.push_back(QPointF(e.x(), mid.y()));
    } else {
        // default: 不画 zig（pts 为空）
    }
    return pts;
}

class TestDiagramPathConnection : public QObject
{
    Q_OBJECT
private slots:
    void path4combo_data();
    void path4combo();
};

void TestDiagramPathConnection::path4combo_data()
{
    QTest::addColumn<DiagramItem::TransformState>("startState");
    QTest::addColumn<DiagramItem::TransformState>("endState");

    // 你要的“4种起点/终点状态组合”
    QTest::newRow("Right->Left")   << DiagramItem::TF_Right  << DiagramItem::TF_Left;
    QTest::newRow("Left->Right")   << DiagramItem::TF_Left   << DiagramItem::TF_Right;
    QTest::newRow("Top->Bottom")   << DiagramItem::TF_Top    << DiagramItem::TF_Bottom;
    QTest::newRow("Bottom->Top")   << DiagramItem::TF_Bottom << DiagramItem::TF_Top;
}

void TestDiagramPathConnection::path4combo()
{
    QFETCH(DiagramItem::TransformState, startState);
    QFETCH(DiagramItem::TransformState, endState);

    QMenu menu;

    // 两个图元（注意：DiagramItem 构造需要 contextMenu）
    auto *startItem = new DiagramItem(DiagramItem::Step, &menu, nullptr);
    auto *endItem   = new DiagramItem(DiagramItem::Step, &menu, nullptr);

    startItem->setFixedSize(QSizeF(140, 90));
    endItem->setFixedSize(QSizeF(140, 90));

    // 故意让它们既不同行也不同列，触发 zig 分支
    startItem->setPos(80, 120);
    endItem->setPos(420, 260);

    QGraphicsScene scene;
    scene.addItem(startItem);
    scene.addItem(endItem);

    // ✅ 你的 DiagramPath 真实构造函数
    auto *pathItem = new DiagramPath(startItem, endItem, startState, endState, nullptr);
    scene.addItem(pathItem);

    // ✅ 必须调用 updatePath() 才会 setPath(m_path)
    pathItem->updatePath();

    const QPainterPath path = pathItem->path();
    QVERIFY2(!path.isEmpty(), "DiagramPath::path() 为空：updatePath() 可能未生效");

    // ----------------- 计算“源码同款”关键点 -----------------
    const QPointF startLink =
        startItem->mapToScene(startItem->linkWhere().value(startState).center());
    const QPointF endLink =
        endItem->mapToScene(endItem->linkWhere().value(endState).center());

    const QPointF startRect =
        startItem->mapToScene(startItem->rectWhere().value(startState).center());
    const QPointF endRect =
        endItem->mapToScene(endItem->rectWhere().value(endState).center());

    // m_state = startState*100 + endState*10 + quad(startLink, endLink)
    const int quad = quadLikeImpl(startLink, endLink);
    const int m_state = int(startState) * 100 + int(endState) * 10 + quad;

    // ----------------- 1) 起点/终点关键点必须出现在路径里 -----------------
    const qreal tol = 1.5;

    QVERIFY2(containsPointFuzzy(path, startRect, tol),
             "路径中未找到 startRectPoint（updatePath: moveTo(startRectPoint)）");
    QVERIFY2(containsPointFuzzy(path, startLink, tol),
             "路径中未找到 startLinkPoint（updatePath: lineTo(startpoint)）");
    QVERIFY2(containsPointFuzzy(path, endLink, tol),
             "路径中未找到 endLinkPoint（updatePath: lineTo(endpoint)）");
    QVERIFY2(containsPointFuzzy(path, endRect, tol),
             "路径中未找到 endRectPoint（updatePath: lineTo(endRectPoint)）");

    // ----------------- 2) Zig 折线分支覆盖：折点必须出现 -----------------
    const QVector<QPointF> zigPts = expectedZigPoints(startLink, endLink, m_state);

    for (const auto& zp : zigPts) {
        QVERIFY2(containsPointFuzzy(path, zp, 2.0),
                 qPrintable(QString("未找到期望折点：(%1,%2)，m_state=%3")
                                .arg(zp.x()).arg(zp.y()).arg(m_state)));
    }

    // ----------------- 3) 轴对齐性（源码只画水平/垂直）-----------------
    // 注意：drawHead 会 moveTo(endRect) 再 lineTo(尖角点)，仍然水平/垂直/斜线？
    // 实际 drawHead 画的是斜线（±5,±5），所以这里我们只对“主干折线段”做弱检查：
    //  - 只要求至少存在一段水平 & 一段垂直，证明 zig 的方向性正确。
    bool hasHorizontal = false;
    bool hasVertical = false;

    for (int i = 1; i < path.elementCount(); ++i) {
        const QPointF a(path.elementAt(i - 1).x, path.elementAt(i - 1).y);
        const QPointF b(path.elementAt(i).x, path.elementAt(i).y);

        if (qFuzzyCompare(a.y(), b.y()) && !qFuzzyCompare(a.x(), b.x())) hasHorizontal = true;
        if (qFuzzyCompare(a.x(), b.x()) && !qFuzzyCompare(a.y(), b.y())) hasVertical = true;
    }

    QVERIFY2(hasHorizontal || zigPts.isEmpty(), "主干路径未检测到水平段（zig 可能不正确）");
    QVERIFY2(hasVertical   || zigPts.isEmpty(), "主干路径未检测到垂直段（zig 可能不正确）");

    // ----------------- 4) 箭头头部存在（至少命中一个尖角点）-----------------
    // drawHead 使用 5 像素偏移画尖角
    // 我们只检查“尖角点”至少出现一个（不强行要求两个都出现，避免浮点/顺序差异）
    const QVector<QPointF> headCandidates = {
        QPointF(endLink.x() - 5, endLink.y() - 5),
        QPointF(endLink.x() - 5, endLink.y() + 5),
        QPointF(endLink.x() + 5, endLink.y() - 5),
        QPointF(endLink.x() + 5, endLink.y() + 5),
    };

    bool foundHead = false;
    for (const auto& hc : headCandidates) {
        if (containsPointFuzzy(path, hc, 2.5)) { foundHead = true; break; }
    }
    QVERIFY2(foundHead, "未检测到箭头尖角点（drawHead 可能未生效或方向不匹配）");

    // 清理
    delete pathItem;
    delete startItem;
    delete endItem;
}

// QTEST_MAIN(TestDiagramPathConnection)
#include "test_diagrampath_connection.moc"
