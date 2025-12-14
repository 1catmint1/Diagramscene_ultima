#include <QtTest/QtTest>
#include <QGraphicsView>
#include <QMenu>
#include <QApplication>
#include <QFont>
#include <QGraphicsTextItem>

#include "../diagramscene.h"
#include "../diagramitem.h"
#include "../diagramtextitem.h"

// 帮助函数：确保视图激活并可交互
static void ensureActive(QGraphicsView& view)
{
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view));
    view.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&view));
    view.setFocus();
    QVERIFY(view.hasFocus());
    view.viewport()->setMouseTracking(true);
}

// 帮助函数：创建一个基础场景和视图
static void makeSceneAndView(QMenu& menu, DiagramScene*& scene, QGraphicsView*& view, int w = 800, int h = 600)
{
    scene = new DiagramScene(&menu);
    scene->setSceneRect(0, 0, w, h);
    view = new QGraphicsView(scene);
    view->resize(w, h);
    ensureActive(*view);
}

// 帮助函数：在指定位置创建一个图元
static DiagramItem* createItemAt(DiagramScene* scene, QMenu* menu, const QPointF& pos)
{
    auto* item = new DiagramItem(DiagramItem::Step, menu);
    item->setPos(pos);
    scene->addItem(item);
    return item;
}

// 测试主体
class TestDiagramTextItemEdit : public QObject
{
    Q_OBJECT
private slots:
    // 用例1：为图元设置“内嵌文字”（通过 DiagramTextItem 作为子项）
    void embedded_label_on_diagram_item_can_be_set_and_styled();

    // 用例2：独立文本框对象作为注释，支持编辑样式与内容
    void standalone_textbox_annotation_can_be_added_and_edited();

    // 用例3：文本框与图元的基本关联与位置关系（将文本框作为子项随图元移动）
    void embedded_label_should_follow_item_when_item_moves();

    // 用例4：文本颜色字段与内容字段的基本一致性（若 DiagramItem 暴露 text_color、textContent）
    void diagramitem_text_fields_are_updateable_when_using_internal_textitem();
};

void TestDiagramTextItemEdit::embedded_label_on_diagram_item_can_be_set_and_styled()
{
    QMenu menu;
    DiagramScene* scene = nullptr;
    QGraphicsView* view = nullptr;
    makeSceneAndView(menu, scene, view);

    // 创建图元
    auto* item = createItemAt(scene, &menu, QPointF(200, 200));
    QVERIFY(item != nullptr);

    // 为该图元创建一个子文本项作为“标签”
    auto* label = new DiagramTextItem;
    QVERIFY(label != nullptr);

    // 设置文字与样式
    label->setPlainText("内嵌标签：处理步骤A");
    QFont f; f.setPointSize(12); f.setBold(true);
    label->setFont(f);
    label->setDefaultTextColor(Qt::blue);

    // 作为子项附着到图元上，并调整相对位置
    label->setParentItem(item);
    label->setPos(0, -20); // 位于图元上方一点点

    // 断言内容与样式
    QCOMPARE(label->toPlainText(), QString("内嵌标签：处理步骤A"));
    QCOMPARE(label->defaultTextColor(), QColor(Qt::blue));
    QCOMPARE(label->font().pointSize(), 12);
    QVERIFY(label->pos().y() <= 0);

    // 清理
    delete view; // 同时销毁 scene
}

void TestDiagramTextItemEdit::standalone_textbox_annotation_can_be_added_and_edited()
{
    QMenu menu;
    DiagramScene* scene = nullptr;
    QGraphicsView* view = nullptr;
    makeSceneAndView(menu, scene, view);

    // 创建一个独立文本框（注释）
    auto* note = new DiagramTextItem;
    QVERIFY(note != nullptr);
    note->setPlainText("独立注释：该步骤需要外部输入。");
    note->setDefaultTextColor(Qt::darkGreen);
    note->setPos(400, 300);
    scene->addItem(note);

    // 编辑独立文本框内容（模拟用户修改）
    note->setPlainText("独立注释：该步骤需要外部输入（已确认）。");

    // 断言：文字内容、样式、位置
    QCOMPARE(note->toPlainText(), QString("独立注释：该步骤需要外部输入（已确认）。"));
    QCOMPARE(note->defaultTextColor(), QColor(Qt::darkGreen));
    QCOMPARE(note->pos(), QPointF(400, 300));

    // 清理
    delete view; // 同时销毁 scene
}

void TestDiagramTextItemEdit::embedded_label_should_follow_item_when_item_moves()
{
    QMenu menu;
    DiagramScene* scene = nullptr;
    QGraphicsView* view = nullptr;
    makeSceneAndView(menu, scene, view);

    auto* item = createItemAt(scene, &menu, QPointF(200, 200));
    QVERIFY(item != nullptr);

    // 创建子文本项并附着
    auto* label = new DiagramTextItem;
    label->setPlainText("绑定标签");
    label->setDefaultTextColor(Qt::black);
    label->setParentItem(item);
    label->setPos(0, -20);

    // 记录初始：label 的场景坐标
    QPointF labelSceneBefore = label->mapToScene(QPointF(0, 0));
    QPointF itemPosBefore = item->pos();

    // 移动图元
    item->setPos(item->pos() + QPointF(120, 80));

    // 再次获取 label 场景坐标
    QPointF labelSceneAfter = label->mapToScene(QPointF(0, 0));
    QPointF itemPosAfter = item->pos();

    // 断言：item 移动了，label 场景坐标也应发生对应变化（随动）
    qreal dLabel = QLineF(labelSceneBefore, labelSceneAfter).length();
    qreal dItem  = QLineF(itemPosBefore, itemPosAfter).length();
    QVERIFY2(dItem > 1.0, "Item did not move enough");
    QVERIFY2(dLabel > 1.0, "Embedded label did not follow item");

    // 清理
    delete view; // 同时销毁 scene
}

void TestDiagramTextItemEdit::diagramitem_text_fields_are_updateable_when_using_internal_textitem()
{
    QMenu menu;
    DiagramScene* scene = nullptr;
    QGraphicsView* view = nullptr;
    makeSceneAndView(menu, scene, view);

    auto* item = createItemAt(scene, &menu, QPointF(250, 220));
    QVERIFY(item != nullptr);

    // 如果 DiagramItem 内部提供 textItem、text_color、textContent，我们在此进行基本一致性验证
    // 注意：这些字段在 diagramitem.h 中已公开（textItem、text_color、textContent）
    // 为 item 创建内部文本框（若构造器未设）
    if (!item->textItem) {
        item->textItem = new QGraphicsTextItem(item);
        item->textItem->setPos(0, 0);
    }

    // 设置内部文本
    item->text_color = QColor(Qt::magenta);
    item->textContent = QString("内部文字：步骤B");
    item->textItem->setDefaultTextColor(item->text_color);
    item->textItem->setPlainText(item->textContent);

    // 断言：内部字段与实际文本项一致
    QCOMPARE(item->textItem->defaultTextColor(), item->text_color);
    QCOMPARE(item->textItem->toPlainText(), item->textContent);

    // 修改并再次断言
    item->text_color = QColor(Qt::darkBlue);
    item->textContent = QString("内部文字：步骤B（更新）");
    item->textItem->setDefaultTextColor(item->text_color);
    item->textItem->setPlainText(item->textContent);

    QCOMPARE(item->textItem->defaultTextColor(), QColor(Qt::darkBlue));
    QCOMPARE(item->textItem->toPlainText(), QString("内部文字：步骤B（更新）"));

    delete view; // 同时销毁 scene
}

// 运行入口
int runDiagramTextItemEditTests(int argc, char** argv)
{
    TestDiagramTextItemEdit tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_diagramtextitem_edit.moc"
