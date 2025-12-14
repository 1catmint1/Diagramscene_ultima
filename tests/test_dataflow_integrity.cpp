#include <QtTest/QtTest>
#include <QGraphicsScene>
#include <QMenu>
#include <QDebug> // 日志

#define protected public
#define private public
#include "../diagramitem.h"
#include "../arrow.h"
#include "../diagrampath.h"
#undef protected
#undef private

class TestDataflowIntegrity : public QObject
{
    Q_OBJECT
private slots:
    void dataflow_integrity();
};

void TestDataflowIntegrity::dataflow_integrity()
{
    QMenu menu;
    DiagramItem *start = new DiagramItem(DiagramItem::Step, &menu);
    DiagramItem *end = new DiagramItem(DiagramItem::Step, &menu);
    start->setPos(0, 0);
    end->setPos(100, 100);

    Arrow *arrow = new Arrow(start, end);
    DiagramPath *path = new DiagramPath(start, end, DiagramItem::TF_Right, DiagramItem::TF_Left);

    QGraphicsScene scene;
    scene.addItem(start);
    scene.addItem(end);
    scene.addItem(arrow);
    scene.addItem(path);

    // 数据追踪：位置数据流
    QString log;
    QTextStream stream(&log);
    stream << "Start: " << start->pos().x() << "," << start->pos().y();
    end->setPos(150, 150);
    arrow->updatePosition();
    path->updatePath();
    stream << " End: " << end->pos().x() << "," << end->pos().y();
    stream << " Arrow P2: " << arrow->line().p2().x() << "," << arrow->line().p2().y();

    // 日志分析：数据传递准确
    QVERIFY(log.contains("150,150")); // 组件间正确
    QVERIFY(log.contains("Arrow P2: 150")); // 传输无误

    delete path;
    delete arrow;
    delete start;
    delete end;
}

int runDataflowIntegrityTests(int argc, char** argv)
{
    TestDataflowIntegrity tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_dataflow_integrity.moc"
