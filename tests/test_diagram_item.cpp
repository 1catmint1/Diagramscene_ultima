#include <QtTest/QtTest>
#include <QMenu>
#include "../diagramitem.h"

class TestDiagramItemCreation : public QObject
{
    Q_OBJECT
private slots:
    void creation_data();
    void creation();

    void size_data();
    void size();

    void stress();
};

void TestDiagramItemCreation::creation_data() {
    QTest::addColumn<DiagramItem::DiagramType>("type");

    QTest::newRow("Step") << DiagramItem::Step;
    QTest::newRow("Conditional") << DiagramItem::Conditional;
    QTest::newRow("StartEnd") << DiagramItem::StartEnd;
    QTest::newRow("Io") << DiagramItem::Io;
    QTest::newRow("circular") << DiagramItem::circular;
    QTest::newRow("Document") << DiagramItem::Document;
    QTest::newRow("PredefinedProcess") << DiagramItem::PredefinedProcess;
    QTest::newRow("StoredData") << DiagramItem::StoredData;
    QTest::newRow("Memory") << DiagramItem::Memory;
    QTest::newRow("SequentialAccessStorage") << DiagramItem::SequentialAccessStorage;
    QTest::newRow("DirectAccessStorage") << DiagramItem::DirectAccessStorage;
    QTest::newRow("Disk") << DiagramItem::Disk;
    QTest::newRow("Card") << DiagramItem::Card;
    QTest::newRow("ManualInput") << DiagramItem::ManualInput;
    QTest::newRow("PerforatedTape") << DiagramItem::PerforatedTape;
    QTest::newRow("Display") << DiagramItem::Display;
    QTest::newRow("Preparation") << DiagramItem::Preparation;
    QTest::newRow("ManualOperation") << DiagramItem::ManualOperation;
    QTest::newRow("ParallelMode") << DiagramItem::ParallelMode;
    QTest::newRow("Hexagon") << DiagramItem::Hexagon;
}

void TestDiagramItemCreation::creation() {
    QFETCH(DiagramItem::DiagramType, type);

    QMenu menu;
    DiagramItem* item = nullptr;
    bool threw = false;

    try {
        item = new DiagramItem(type, &menu, nullptr);
    } catch (...) {
        threw = true;
    }

    QVERIFY2(!threw, qPrintable(QString("构造抛异常 type=%1").arg(int(type))));
    QVERIFY2(item != nullptr, qPrintable(QString("构造返回空指针 type=%1").arg(int(type))));

    if (item) {
        QCOMPARE(item->diagramType(), type);
        QCOMPARE(item->type(), DiagramItem::Type);
    }

    delete item;
}

void TestDiagramItemCreation::size_data() {
    QTest::addColumn<QSizeF>("size");

    QTest::newRow("normal") << QSizeF(150.0, 100.0);
    QTest::newRow("min_small") << QSizeF(1.0, 1.0);
    QTest::newRow("large") << QSizeF(10000.0, 8000.0);
    QTest::newRow("negative") << QSizeF(-10.0, -5.0);
}

void TestDiagramItemCreation::size() {
    QFETCH(QSizeF, size);

    QMenu menu;
    auto* item = new DiagramItem(DiagramItem::Step, &menu, nullptr);

    item->setFixedSize(size);
    QCOMPARE(item->getSize(), size);

    delete item;
}

void TestDiagramItemCreation::stress() {
    QMenu menu;
    for (int i = 0; i < 200; ++i) {
        auto* it = new DiagramItem(DiagramItem::Step, &menu, nullptr);
        it->setFixedSize(QSizeF(100 + i % 50, 80 + i % 30));
        delete it;
    }
}

// 关键：提供给 test_main.cpp 链接的符号
int runDiagramItemTests(int argc, char** argv)
{
    TestDiagramItemCreation tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_diagram_item.moc"
