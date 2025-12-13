#include <QtTest/QtTest>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QSignalSpy>

#include "../diagramtextitem.h"

// --------- 工具函数：scene → view 坐标 ----------
static QPoint viewPos(QGraphicsView *view, const QPointF &scenePt)
{
    return view->mapFromScene(scenePt);
}

static void dblClickOnItemCenter(QGraphicsView *view, QGraphicsItem *item)
{
    QVERIFY(view);
    QVERIFY(item);
    const QPoint vp = viewPos(view, item->sceneBoundingRect().center());
    QTest::mouseDClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, vp);
}

static void clickOnItemCenter(QGraphicsView *view, QGraphicsItem *item)
{
    QVERIFY(view);
    QVERIFY(item);
    const QPoint vp = viewPos(view, item->sceneBoundingRect().center());
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, vp);
}

// 在 scene 中创建一个“可聚焦 + 吃鼠标”的焦点接收器
static QGraphicsRectItem* addFocusSink(QGraphicsScene &scene, const QRectF &rect)
{
    auto *sink = scene.addRect(rect);
    sink->setFlag(QGraphicsItem::ItemIsFocusable, true);
    sink->setAcceptedMouseButtons(Qt::LeftButton);
    sink->setZValue(9999); // 防止被其它 item 遮住点击
    return sink;
}

class TestDiagramTextItemEdit : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void doubleClick_enters_edit_mode();
    void typing_and_editing_works();

    void focus_out_closes_edit_and_emits_signal();
    void doubleClick_reenters_edit_after_focus_out();
};

// ------------------------------------------------
// T1：双击进入编辑模式
// ------------------------------------------------
void TestDiagramTextItemEdit::doubleClick_enters_edit_mode()
{
    DiagramTextItem *textItem = new DiagramTextItem;

    QGraphicsScene scene;
    scene.addItem(textItem);
    textItem->setPos(100, 80);

    QGraphicsView view(&scene);
    view.resize(600, 400);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    // 初始应当是可编辑（构造函数已开启）
    QCOMPARE(textItem->textInteractionFlags(), Qt::TextEditorInteraction);

    // 人为关闭编辑，验证双击能恢复
    textItem->setTextInteractionFlags(Qt::NoTextInteraction);
    QCOMPARE(textItem->textInteractionFlags(), Qt::NoTextInteraction);

    dblClickOnItemCenter(&view, textItem);

    QCOMPARE(textItem->textInteractionFlags(), Qt::TextEditorInteraction);

    delete textItem;
}

// ------------------------------------------------
// T2：编辑状态下键盘输入 / 删除正常
// ------------------------------------------------
void TestDiagramTextItemEdit::typing_and_editing_works()
{
    DiagramTextItem *textItem = new DiagramTextItem;

    QGraphicsScene scene;
    scene.addItem(textItem);
    textItem->setPos(120, 100);

    QGraphicsView view(&scene);
    view.resize(600, 400);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    // 双击进入编辑
    dblClickOnItemCenter(&view, textItem);
    QCOMPARE(textItem->textInteractionFlags(), Qt::TextEditorInteraction);

    // Ctrl+A → 输入文本
    QTest::keyClick(view.viewport(), Qt::Key_A, Qt::ControlModifier);
    QTest::keyClicks(view.viewport(), "Hello");
    QCoreApplication::processEvents();

    QCOMPARE(textItem->toPlainText(), QString("Hello"));

    // Backspace 删除最后一个字符
    QTest::keyClick(view.viewport(), Qt::Key_Backspace);
    QCoreApplication::processEvents();
    QCOMPARE(textItem->toPlainText(), QString("Hell"));

    // Ctrl+A + Delete 清空
    QTest::keyClick(view.viewport(), Qt::Key_A, Qt::ControlModifier);
    QTest::keyClick(view.viewport(), Qt::Key_Delete);
    QCoreApplication::processEvents();
    QCOMPARE(textItem->toPlainText(), QString(""));

    delete textItem;
}

// ------------------------------------------------
// T3：失焦 → 关闭编辑 + 发 lostFocus 信号
// （用 focus sink 接收点击，保证鼠标事件被接受，从而触发失焦）
// ------------------------------------------------
void TestDiagramTextItemEdit::focus_out_closes_edit_and_emits_signal()
{
    DiagramTextItem *textItem = new DiagramTextItem;

    QGraphicsScene scene;
    scene.addItem(textItem);
    textItem->setPos(140, 120);

    QGraphicsView view(&scene);
    view.resize(600, 400);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    QSignalSpy spyLost(textItem, &DiagramTextItem::lostFocus);

    // 进入编辑模式
    dblClickOnItemCenter(&view, textItem);
    QCOMPARE(textItem->textInteractionFlags(), Qt::TextEditorInteraction);
    QVERIFY(scene.focusItem() == textItem);

    // ⭐ 核心：直接清空 scene 焦点
    scene.clearFocus();
    QCoreApplication::processEvents();

    // 现在一定会触发 focusOutEvent
    QTRY_COMPARE(textItem->textInteractionFlags(), Qt::NoTextInteraction);
    QTRY_COMPARE(spyLost.count(), 1);

    delete textItem;
}


// ------------------------------------------------
// T4：失焦后可再次双击进入编辑
// ------------------------------------------------
void TestDiagramTextItemEdit::doubleClick_reenters_edit_after_focus_out()
{
    DiagramTextItem *textItem = new DiagramTextItem;

    QGraphicsScene scene;
    scene.addItem(textItem);
    textItem->setPos(160, 140);

    QGraphicsView view(&scene);
    view.resize(600, 400);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));

    // 进入编辑
    dblClickOnItemCenter(&view, textItem);
    QCOMPARE(textItem->textInteractionFlags(), Qt::TextEditorInteraction);

    // 失焦
    scene.clearFocus();
    QCoreApplication::processEvents();
    QCOMPARE(textItem->textInteractionFlags(), Qt::NoTextInteraction);

    // 再次双击 → 重新进入编辑
    dblClickOnItemCenter(&view, textItem);
    QCOMPARE(textItem->textInteractionFlags(), Qt::TextEditorInteraction);

    delete textItem;
}
int runDiagramTextItemEditTests(int argc, char** argv)
{
    TestDiagramTextItemEdit tc;
    return QTest::qExec(&tc, argc, argv);
}
#include "test_diagramtextitem_edit.moc"
