#include <QtTest/QtTest>
#include <QVariantMap>
#include <QMenu>
#include "../diagramitem.h"   // 相对路径按你的项目结构调整（tests 在工程根同级）

class TestDiagramItemCreation : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {
        // 全局初始化（如果需要）
    }

    void cleanupTestCase() {
        // 全局清理（如果需要）
    }

    // ---------------- Creation tests ----------------
    void creation_data() {
        QTest::addColumn<DiagramItem::DiagramType>("type");
        QTest::addColumn<bool>("expectCreated");

        // 列出 DiagramType 中的每一种（与 diagramitem.h 中 enum 顺序一致）
        QTest::newRow("Step") << DiagramItem::Step << true;
        QTest::newRow("Conditional") << DiagramItem::Conditional << true;
        QTest::newRow("StartEnd") << DiagramItem::StartEnd << true;
        QTest::newRow("Io") << DiagramItem::Io << true;
        QTest::newRow("circular") << DiagramItem::circular << true;
        QTest::newRow("Document") << DiagramItem::Document << true;
        QTest::newRow("PredefinedProcess") << DiagramItem::PredefinedProcess << true;
        QTest::newRow("StoredData") << DiagramItem::StoredData << true;
        QTest::newRow("Memory") << DiagramItem::Memory << true;
        QTest::newRow("SequentialAccessStorage") << DiagramItem::SequentialAccessStorage << true;
        QTest::newRow("DirectAccessStorage") << DiagramItem::DirectAccessStorage << true;
        QTest::newRow("Disk") << DiagramItem::Disk << true;
        QTest::newRow("Card") << DiagramItem::Card << true;
        QTest::newRow("ManualInput") << DiagramItem::ManualInput << true;
        QTest::newRow("PerforatedTape") << DiagramItem::PerforatedTape << true;
        QTest::newRow("Display") << DiagramItem::Display << true;
        QTest::newRow("Preparation") << DiagramItem::Preparation << true;
        QTest::newRow("ManualOperation") << DiagramItem::ManualOperation << true;
        QTest::newRow("ParallelMode") << DiagramItem::ParallelMode << true;
        QTest::newRow("Hexagon") << DiagramItem::Hexagon << true;
    }

    void creation() {
        QFETCH(DiagramItem::DiagramType, type);
        QFETCH(bool, expectCreated);

        // DiagramItem 构造需要一个 QMenu*（context menu），传入一个临时 QMenu
        QMenu *menu = new QMenu(nullptr);
        DiagramItem *item = nullptr;
        bool threw = false;
        try {
            item = new DiagramItem(type, menu, nullptr);
        } catch (...) {
            threw = true;
        }

        // 基本验证：对象被创建，类型标识正确，且 type() 返回自定义 Type
        QVERIFY2(!threw, qPrintable(QString("构造抛异常 type=%1").arg(int(type))));
        QVERIFY2(item != nullptr, qPrintable(QString("构造返回空指针 type=%1").arg(int(type))));
        if (item) {
            QCOMPARE(item->diagramType(), type);      // 枚举值应当匹配
            QCOMPARE(item->type(), DiagramItem::Type); // QGraphicsItem::type() 应返回 Type
        }

        // 清理
        delete item;
        delete menu;
    }

    // ---------------- Size / boundary tests（对单个类型做边界/稳定性检查） ----------------
    void size_data() {
        QTest::addColumn<QSizeF>("size");
        QTest::addColumn<bool>("expectNoCrash");

        QTest::newRow("normal") << QSizeF(150.0, 100.0) << true;
        QTest::newRow("min_small") << QSizeF(1.0, 1.0) << true;
        QTest::newRow("large") << QSizeF(10000.0, 8000.0) << true;
        QTest::newRow("negative") << QSizeF(-10.0, -5.0) << true; // 期望不崩溃、稳定（具体行为视实现而定）
    }

    void size() {
        QFETCH(QSizeF, size);
        QFETCH(bool, expectNoCrash);

        DiagramItem::DiagramType t = DiagramItem::Step; // 任意一种类型用于尺寸测试
        QMenu *menu = new QMenu(nullptr);
        DiagramItem *item = nullptr;
        bool threw = false;
        try {
            item = new DiagramItem(t, menu, nullptr);
            // 使用 setFixedSize 测试边界
            item->setFixedSize(size);
        } catch (...) {
            threw = true;
        }

        QVERIFY2(expectNoCrash ? !threw : true, qPrintable(QString("尺寸操作导致异常 size=%1x%2").arg(size.width()).arg(size.height())));
        if (item) {
            // 验证 getSize 与刚才设置的值一致（当前实现直接赋值）
            QSizeF actual = item->getSize();
            QCOMPARE(actual, size);
        }

        delete item;
        delete menu;
    }

    // ---------------- Stability test: repeated creation/destruction ----------------
    void stress() {
        QMenu menu; // 栈上一个 QMenu
        for (int i = 0; i < 200; ++i) {
            DiagramItem *it = nullptr;
            bool threw = false;
            try {
                it = new DiagramItem(DiagramItem::Step, &menu, nullptr);
                it->setFixedSize(QSizeF(100 + i % 50, 80 + i % 30));
            } catch (...) {
                threw = true;
            }
            QVERIFY2(!threw, "反复创建/销毁过程中发生异常");
            delete it;
        }
    }
};

int runDiagramItemCreationTests(int argc, char** argv)
{
    TestDiagramItemCreation tc;
    return QTest::qExec(&tc, argc, argv);
}
#include "test_diagramitems_create.moc"
