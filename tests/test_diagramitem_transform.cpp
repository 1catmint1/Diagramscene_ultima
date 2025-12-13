#include <QtTest/QtTest>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>

#define protected public
#define private public
#include "../diagramitem.h"
#undef protected
#undef private

class TestDiagramItemTransform : public QObject
{
    Q_OBJECT
private slots:
    void transform_data();
    void transform();
};

void TestDiagramItemTransform::transform_data()
{
    QTest::addColumn<int>("state");   // DiagramItem::TransformState -> int
    QTest::addColumn<qreal>("fx");    // x' = x0 + fx * dx
    QTest::addColumn<qreal>("fw");    // w' = w0 + fw * dx
    QTest::addColumn<qreal>("fy");    // y' = y0 + fy * dy
    QTest::addColumn<qreal>("fh");    // h' = h0 + fh * dy

    // 约定：dx > 0, dy > 0
    // 根据 mouseMoveEvent 中的算法推导：
    // 右：    w = w + dx
    // 左：    x = x + dx; w = w - dx
    // 下：    h = h + dy
    // 上：    y = y + dy; h = h - dy
    // 角：组合上述两个方向

    QTest::newRow("Right")
        << int(DiagramItem::TF_Right)
        << 0.0  // x 不变
        << 1.0  // w + dx
        << 0.0  // y 不变
        << 0.0; // h 不变

    QTest::newRow("Left")
        << int(DiagramItem::TF_Left)
        << 1.0   // x + dx
        << -1.0  // w - dx
        << 0.0
        << 0.0;

    QTest::newRow("Bottom")
        << int(DiagramItem::TF_Bottom)
        << 0.0
        << 0.0
        << 0.0
        << 1.0;  // h + dy

    QTest::newRow("Top")
        << int(DiagramItem::TF_Top)
        << 0.0
        << 0.0
        << 1.0   // y + dy
        << -1.0; // h - dy

    QTest::newRow("TopLeft")
        << int(DiagramItem::TF_TopL)
        << 1.0   // x + dx
        << -1.0  // w - dx
        << 1.0   // y + dy
        << -1.0; // h - dy

    QTest::newRow("TopRight")
        << int(DiagramItem::TF_TopR)
        << 0.0   // x 不变
        << 1.0   // w + dx
        << 1.0   // y + dy
        << -1.0; // h - dy

    QTest::newRow("BottomLeft")
        << int(DiagramItem::TF_BottomL)
        << 1.0   // x + dx
        << -1.0  // w - dx
        << 0.0
        << 1.0;  // h + dy

    QTest::newRow("BottomRight")
        << int(DiagramItem::TF_BottomR)
        << 0.0
        << 1.0   // w + dx
        << 0.0
        << 1.0;  // h + dy
}

void TestDiagramItemTransform::transform()
{
    QFETCH(int, state);
    QFETCH(qreal, fx);
    QFETCH(qreal, fw);
    QFETCH(qreal, fy);
    QFETCH(qreal, fh);

    // 初始位置 / 尺寸（保持一致，便于计算）
    const qreal x0 = 100.0;
    const qreal y0 = 50.0;
    const qreal w0 = 200.0;
    const qreal h0 = 100.0;

    // 鼠标移动偏移量（本地坐标）
    const qreal dx = 10.0;
    const qreal dy = 20.0;

    QMenu menu;
    DiagramItem *item = new DiagramItem(DiagramItem::Step, &menu, nullptr);
    item->setFixedSize(QSizeF(w0, h0));
    item->setPos(x0, y0);

    // 确保允许变换（isChange = true）
    item->ableEvents();   // 里头会把 isChange 设为 true

    // 直接设定当前的 TransformState（通过 private→public 的小技巧访问）
    item->m_tfState = static_cast<DiagramItem::TransformState>(state);

    // 构造一个鼠标移动事件，使得 pos - lastPos = (dx, dy)
    QGraphicsSceneMouseEvent ev(QEvent::GraphicsSceneMouseMove);
    ev.setButtons(Qt::LeftButton);
    ev.setPos(QPointF(dx, dy));
    ev.setLastPos(QPointF(0.0, 0.0));

    // 调用被测函数
    item->mouseMoveEvent(&ev);

    // 计算期望的坐标和尺寸
    qreal expX = x0 + fx * dx;
    qreal expY = y0 + fy * dy;
    qreal expW = w0 + fw * dx;
    qreal expH = h0 + fh * dy;

    // mouseMoveEvent 中有宽高最小为 40 的约束，这里也同步应用
    if (expW < 40.0) expW = 40.0;
    if (expH < 40.0) expH = 40.0;

    // 取实际结果
    QPointF pos = item->pos();
    QSizeF size = item->getSize();   // getSize() 返回 m_grapSize

    auto fuzzyEqual = [](qreal a, qreal b) {
        return qFuzzyCompare(1.0 + a, 1.0 + b);
    };

    QVERIFY2(fuzzyEqual(pos.x(), expX),
             qPrintable(QString("X 坐标不匹配: actual=%1 expected=%2")
                            .arg(pos.x()).arg(expX)));

    QVERIFY2(fuzzyEqual(pos.y(), expY),
             qPrintable(QString("Y 坐标不匹配: actual=%1 expected=%2")
                            .arg(pos.y()).arg(expY)));

    QVERIFY2(fuzzyEqual(size.width(), expW),
             qPrintable(QString("宽度不匹配: actual=%1 expected=%2")
                            .arg(size.width()).arg(expW)));

    QVERIFY2(fuzzyEqual(size.height(), expH),
             qPrintable(QString("高度不匹配: actual=%1 expected=%2")
                            .arg(size.height()).arg(expH)));

    delete item;
}

int runDiagramItemTransformTests(int argc, char** argv)
{
    TestDiagramItemTransform tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_diagramitem_transform.moc"
