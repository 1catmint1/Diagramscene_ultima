#include <QtTest/QtTest>
#include <QMenu>
#include <QGraphicsView>
#include <QUndoStack>

#include "diagramscene.h"
#include "diagramitem.h"
#include "deletecommand.h"

static int countDiagramItems(QGraphicsScene* scene)
{
    int c = 0;
    for (QGraphicsItem* gi : scene->items())
        if (gi->type() == DiagramItem::Type) ++c;
    return c;
}

class TestUndoRedo : public QObject
{
    Q_OBJECT
private slots:
    void deleteCommand_undo_redo();
};

void TestUndoRedo::deleteCommand_undo_redo()
{
    QMenu dummyMenu;
    DiagramScene scene(&dummyMenu);

    auto* item = new DiagramItem(DiagramItem::Step, &dummyMenu);
    item->setPos(123, 456);
    scene.addItem(item);

    QCOMPARE(countDiagramItems(&scene), 1);

    QUndoStack stack;
    stack.push(new DeleteCommand(item, &scene)); // push 会调用 redo()

    // redo => removeItem
    QCOMPARE(countDiagramItems(&scene), 0);

    // undo => addItem + pos restore
    stack.undo();
    QCOMPARE(countDiagramItems(&scene), 1);
    QCOMPARE(item->pos(), QPointF(123, 456));

    // redo => removeItem
    stack.redo();
    QCOMPARE(countDiagramItems(&scene), 0);
}

int runUndoRedoTests(int argc, char** argv)
{
    TestUndoRedo tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_undo_redo.moc"
