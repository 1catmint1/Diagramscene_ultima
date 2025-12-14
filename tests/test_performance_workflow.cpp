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

class TestPerformanceWorkflow : public QObject
{
    Q_OBJECT
private slots:
    void performance_workflow();
};

void TestPerformanceWorkflow::performance_workflow()
{
    QMenu menu;
    DiagramScene scene(&menu);
    QGraphicsView view(&scene);

    QElapsedTimer timer;
    timer.start();

    // 自动化脚本：模拟业务流程（插入、移动、更新）
    for (int i = 0; i < 10; ++i) { // 小规模验证可重现
        DiagramItem *item = new DiagramItem(DiagramItem::Step, &menu);
        scene.addItem(item);
        item->setPos(i * 10, i * 10);
    }
    scene.update();

    qint64 elapsed = timer.elapsed();
    QVERIFY(elapsed < 50); // 流程响应快，确保100%可重现

    // 录屏工具模拟：假设QTest::qWaitForWindowExposed(view); 但这里用时间验证

    view.show(); // 确保可视化流程
    QTest::qWait(100); // 模拟录屏延迟
}

int runPerformanceWorkflowTests(int argc, char** argv)
{
    TestPerformanceWorkflow tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_performance_workflow.moc"
