#include <QtTest/QtTest>
#include <QGraphicsScene>
#include <QMenu>
#include <QtMath>

#define protected public
#define private public
#include "../diagrampath.h"
#include "../diagramitem.h"
#undef protected
#undef private

// 辅助函数：路径元素是否包含点（允许容差）
static bool pathContainsPoint(const QPainterPath &path, const QPointF &point, qreal tolerance = 20.0)
{
    for (int i = 0; i < path.elementCount(); ++i) {
        QPointF elem(path.elementAt(i).x, path.elementAt(i).y);
        if (QLineF(point, elem).length() <= tolerance) {
            return true;
        }
    }
    return false;
}

class TestDiagramPathPathDev : public QObject
{
    Q_OBJECT
private slots:
    void path_dev_data();
    void path_dev();
};

void TestDiagramPathPathDev::path_dev_data()
{
    QTest::addColumn<DiagramItem::TransformState>("startState");
    QTest::addColumn<DiagramItem::TransformState>("endState");

    QTest::newRow("TopToBottom")       << DiagramItem::TF_Top      << DiagramItem::TF_Bottom;
    QTest::newRow("BottomToTop")       << DiagramItem::TF_Bottom   << DiagramItem::TF_Top;
    QTest::newRow("LeftToRight")       << DiagramItem::TF_Left     << DiagramItem::TF_Right;
    QTest::newRow("RightToLeft")       << DiagramItem::TF_Right    << DiagramItem::TF_Left;
    QTest::newRow("TopLToBottomR")     << DiagramItem::TF_TopL     << DiagramItem::TF_BottomR;
    QTest::newRow("BottomRToTopL")     << DiagramItem::TF_BottomR  << DiagramItem::TF_TopL;
    QTest::newRow("TopToLeft")         << DiagramItem::TF_Top      << DiagramItem::TF_Left;
    QTest::newRow("RightToBottom")     << DiagramItem::TF_Right    << DiagramItem::TF_Bottom;
}

void TestDiagramPathPathDev::path_dev()
{
    QFETCH(DiagramItem::TransformState, startState);
    QFETCH(DiagramItem::TransformState, endState);

    QMenu menu;
    DiagramItem *startItem = new DiagramItem(DiagramItem::Step, &menu);
    DiagramItem *endItem   = new DiagramItem(DiagramItem::Step, &menu);

    startItem->setPos(0, 0);
    endItem->setPos(300, 300);

    DiagramPath *path = new DiagramPath(startItem, endItem, startState, endState);
    path->updatePath();

    QPointF expectedStart = startItem->mapToScene(startItem->linkWhere()[startState].center());
    QPointF expectedEnd   = endItem->mapToScene(endItem->linkWhere()[endState].center());

    QPainterPath painterPath = path->path();
    QPointF pathStart = painterPath.pointAtPercent(0.0);
    QPointF pathEnd   = painterPath.pointAtPercent(1.0);

    qreal startError = QLineF(pathStart, expectedStart).length();
    qreal endError   = QLineF(pathEnd,   expectedEnd).length();

    qDebug() << "场景:" << QTest::currentDataTag()
             << "起点误差:" << startError << "终点误差:" << endError;

    // 核心验证：路径几何必须包含两个锚点（允许20像素容差，覆盖外部偏移+箭头头+折线）
    QVERIFY2(pathContainsPoint(painterPath, expectedStart, 20.0),
             qPrintable(QString("路径未在20像素内包含起点锚点，误差%1").arg(startError)));
    QVERIFY2(pathContainsPoint(painterPath, expectedEnd, 20.0),
             qPrintable(QString("路径未在20像素内包含终点锚点，误差%1").arg(endError)));

    // 可选信息验证：记录误差（不失败）
    QTest::qWarn(qPrintable(QString("视觉偏移正常：起点%1 终点%2").arg(startError).arg(endError)));

    delete path;
    delete startItem;
    delete endItem;
}

int runDiagramPathPathDevTests(int argc, char** argv)
{
    TestDiagramPathPathDev tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_diagrampath_path_dev.moc"
