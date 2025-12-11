#include <QtTest/QtTest>
#include <QImage>
#include <QPainter>
#include <QMenu>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <numeric>

#include "../diagramitem.h" // 按你的工程结构调整相对路径

// =================== 工具函数 ===================

// 计算区域内平均颜色（目前只在填充色测试里用）
static QColor averageColor(const QImage &img, const QRect &rect)
{
    long long r = 0, g = 0, b = 0;
    int count = 0;

    for (int y = rect.top(); y <= rect.bottom(); ++y) {
        for (int x = rect.left(); x <= rect.right(); ++x) {
            if (!img.rect().contains(x, y))
                continue;
            const QColor c = img.pixelColor(x, y);
            r += c.red();
            g += c.green();
            b += c.blue();
            ++count;
        }
    }

    if (count == 0)
        return QColor(0, 0, 0);

    return QColor(int(r / count), int(g / count), int(b / count));
}

// 颜色相似度（容差）
static bool colorAlmostEqual(const QColor &a, const QColor &b, int tol = 120)
{
    return qAbs(a.red()   - b.red())   <= tol &&
           qAbs(a.green() - b.green()) <= tol &&
           qAbs(a.blue()  - b.blue())  <= tol;
}

// 把 scene 坐标映射到 image 坐标（用于精确采样）
static QPoint scenePointToImagePoint(const QRectF &source,
                                     const QSize &imgSize,
                                     const QPointF &pt)
{
    const qreal sx = (pt.x() - source.left()) / source.width();
    const qreal sy = (pt.y() - source.top())  / source.height();

    const int ix = qRound(sx * (imgSize.width()  - 1));
    const int iy = qRound(sy * (imgSize.height() - 1));

    return QPoint(ix, iy);
}

// =================== 测试类 ===================

class TestDiagramItemProperties : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase()    { }
    void cleanupTestCase() { }

    // ---------- 1. 填充色渲染测试 ----------
    //
    //  - 测 setBrush(m_color)
    //  - 测 paint() 在未选中状态下使用 m_color 填充
    //
    void fillColor_rendering()
    {
        QMenu menu;
        DiagramItem *item = new DiagramItem(DiagramItem::Step, &menu, nullptr);
        item->setFixedSize(QSizeF(120, 80));

        QColor fillColor = Qt::red;
        item->setBrush(fillColor);

        // 白盒断言：公共成员 m_color 已同步更新
        QCOMPARE(item->m_color, fillColor);

        QGraphicsScene scene;
        scene.addItem(item);
        item->setPos(30, 30);

        const QRectF source = item->sceneBoundingRect().adjusted(-1, -1, 1, 1);

        const QSize imgSize(480, 360);
        QImage img(imgSize, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::green);  // 背景设为绿，用于区分

        QPainter painter(&img);
        painter.setRenderHint(QPainter::Antialiasing, false); // 禁用抗锯齿方便取色
        scene.render(&painter,
                     QRectF(0, 0, imgSize.width(), imgSize.height()),
                     source);
        painter.end();

        const QPointF sceneCenter = item->sceneBoundingRect().center();
        const QPoint center = scenePointToImagePoint(source, imgSize, sceneCenter);

        const QRect avgRect(center.x() - 4, center.y() - 4, 9, 9);
        const QColor avg = averageColor(img, avgRect);

        QVERIFY2(colorAlmostEqual(avg, fillColor, 120),
                 qPrintable(QStringLiteral("平均颜色 %1,%2,%3 与期望红色差距过大")
                                .arg(avg.red()).arg(avg.green()).arg(avg.blue())));

        scene.removeItem(item);
        delete item;
    }

    // ---------- 2. 选中状态下的边框高亮分支（if 为 true） ----------
    //
    //  - ableEvents() → isHover = true, isChange = true
    //  - setSelected(true) → isSelected() == true
    //  - paint() 中 isSelected && isHover && isChange 为真，进入边框绘制分支
    //
    void selectionBorder_state()
    {
        QMenu menu;
        DiagramItem *item = new DiagramItem(DiagramItem::Step, &menu, nullptr);
        item->setFixedSize(QSizeF(160, 100));

        QColor fillColor = Qt::yellow;
        item->setBrush(fillColor);

        // 启用事件，使 isHover / isChange 为 true
        item->ableEvents();

        item->setSelected(true);
        QVERIFY(item->isSelected());

        // 记录几何信息
        const QRectF brBefore = item->boundingRect();
        const QSizeF sizeBefore = item->getSize();

        QGraphicsScene scene;
        scene.addItem(item);
        item->setPos(20, 20);

        const QRectF source = item->sceneBoundingRect().adjusted(-1, -1, 1, 1);
        const QSize imgSize(400, 300);
        QImage img(imgSize, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);

        QPainter painter(&img);
        painter.setRenderHint(QPainter::Antialiasing, false);
        scene.render(&painter,
                     QRectF(0, 0, imgSize.width(), imgSize.height()),
                     source);
        painter.end();

        // 行为断言：选中不应改变填充色和几何信息
        QCOMPARE(item->m_color, fillColor);
        QCOMPARE(item->boundingRect(), brBefore);
        QCOMPARE(item->getSize(), sizeBefore);

        scene.removeItem(item);
        delete item;
    }

    // ---------- 3. disableEvents 后不再进入高亮分支（if 为 false） ----------
    //
    //  - disableEvents() → isHover = false, isChange = false
    //  - setSelected(true) 但因为 isHover/isChange 为 false，高亮分支不会执行
    //  - 尽管如此，填充色仍然照常生效（paint() 仍调用 painter->setBrush(m_color)）
    //
    void selectionBorder_disableEvents()
    {
        QMenu menu;
        DiagramItem *item = new DiagramItem(DiagramItem::Step, &menu, nullptr);
        item->setFixedSize(QSizeF(160, 100));

        QColor fillColor = Qt::cyan;
        item->setBrush(fillColor);

        // 禁用事件：isHover = false, isChange = false
        item->disableEvents();

        item->setSelected(true);
        QVERIFY(item->isSelected());

        QGraphicsScene scene;
        scene.addItem(item);
        item->setPos(20, 20);

        const QRectF source = item->sceneBoundingRect().adjusted(-1, -1, 1, 1);
        const QSize imgSize(400, 300);
        QImage img(imgSize, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);

        QPainter painter(&img);
        painter.setRenderHint(QPainter::Antialiasing, false);
        scene.render(&painter,
                     QRectF(0, 0, imgSize.width(), imgSize.height()),
                     source);
        painter.end();

        // 这里我们不强行做像素级边框检测，只验证：
        //  - setBrush 的颜色仍然保留
        //  - 选中 + disableEvents 不会引起崩溃
        QCOMPARE(item->m_color, fillColor);

        scene.removeItem(item);
        delete item;
    }

    // ---------- 4. 尺寸属性链测试 ----------
    void size_property_chain()
    {
        QMenu menu;
        DiagramItem *item = new DiagramItem(DiagramItem::Step, &menu, nullptr);

        const QSizeF s(250.5, 180.25);
        item->setFixedSize(s);

        QCOMPARE(item->getSize(), s);

        delete item;
    }

    // ---------- 5. 文本属性链测试 ----------
    void text_property_chain()
    {
        QMenu menu;
        DiagramItem *item = new DiagramItem(DiagramItem::Step, &menu, nullptr);

        QVERIFY(item->textItem != nullptr);

        const QString text = QStringLiteral("单元测试");
        item->textItem->setPlainText(text);
        QCOMPARE(item->textItem->toPlainText(), text);

        const QColor tcolor = Qt::blue;
        item->textItem->setDefaultTextColor(tcolor);
        QCOMPARE(item->textItem->defaultTextColor(), tcolor);

        delete item;
    }
};

// QTEST_MAIN(TestDiagramItemProperties)
#include "test_diagramitem_properties.moc"
