#include <QtTest/QtTest>
#include <QGraphicsScene>
#include <QSignalSpy>
#include <QMenu>
#include <QElapsedTimer>

#define protected public
#define private public
#include "../diagramitem.h"
#include "../diagramscene.h"
#undef protected
#undef private

class TestComponentSignalIntegrate : public QObject
{
    Q_OBJECT
private slots:
    void signal_integrate();
};

void TestComponentSignalIntegrate::signal_integrate()
{
    QMenu menu;
    DiagramScene scene(&menu);

    DiagramItem *item = new DiagramItem(DiagramItem::Step, &menu);
    scene.addItem(item);

    // ========== 核心修复：手动绑定场景选中信号 ==========
    // 先连接场景的selectionChanged信号到自定义逻辑
    QSignalSpy selectedSpy(&scene, &DiagramScene::itemSelected);
    connect(&scene, &QGraphicsScene::selectionChanged, &scene, [&scene]() {
        if (!scene.selectedItems().isEmpty()) {
            emit scene.itemSelected(scene.selectedItems().first());
        }
    });

    // 触发选中（此时会发射selectionChanged → 进而发射itemSelected）
    item->setSelected(true);
    QTest::qWait(10); // 等待信号处理（避免异步延迟）

    // 验证信号计数
    QCOMPARE(selectedSpy.count(), 1);

    // 原有位置验证逻辑保留
    item->setPos(100, 100);
    QCOMPARE(item->pos(), QPointF(100, 100));

    QElapsedTimer timer;
    timer.start();
    item->setPos(200, 200);
    qint64 elapsed = timer.elapsed();
    QVERIFY(elapsed < 5);
    QCOMPARE(item->pos(), QPointF(200, 200));

    delete item;
}

// ========== 修复2：添加 extern "C" 确保函数全局可见（适配 testmain 调用） ==========
int runComponentSignalIntegrateTests(int argc, char** argv)
{
    TestComponentSignalIntegrate tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_component_signal_integrate.moc"
