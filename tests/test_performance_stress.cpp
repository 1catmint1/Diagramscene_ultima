#include <QtTest/QtTest>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMenu>
#include <QElapsedTimer>

#define protected public
#define private public
#include "../diagramscene.h"
#include "../diagramitem.h"
#undef protected
#undef private

class TestPerformanceStress : public QObject
{
    Q_OBJECT
private slots:
    void performance_stress();
};

void TestPerformanceStress::performance_stress()
{
    QMenu menu;
    DiagramScene scene(&menu);
    QGraphicsView view(&scene);

    // 负载生成：1000个项
    for (int i = 0; i < 1000; ++i) {
        DiagramItem *item = new DiagramItem(DiagramItem::Step, &menu);
        scene.addItem(item);
        item->setPos(i % 100 * 10, i / 100 * 10);
    }

    QElapsedTimer timer;
    timer.start();

    // 性能分析：更新场景
    scene.update();
    view.viewport()->update();

    qint64 elapsed = timer.elapsed();
    QVERIFY2(elapsed < 100, qPrintable(QString("Stress elapsed: %1ms").arg(elapsed))); // <100ms
}

int runPerformanceStressTests(int argc, char** argv)
{
    TestPerformanceStress tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_performance_stress.moc"
