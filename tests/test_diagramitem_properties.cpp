#include <QtTest/QtTest>
#include <QImage>
#include <QPainter>
#include <QMenu>
#include <QGraphicsScene>
#include <QGraphicsView>

#include "../diagramitem.h"

// =================== 工具函数 ===================

// 计算区域内平均颜色
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

// 把 scene 坐标映射到 image 坐标
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

    void fillColor_rendering();
    void selectionBorder_state();
    void selectionBorder_disableEvents();
    void size_property_chain();
    void text_property_chain();
};

void TestDiagramItemProperties::fillColor_rendering()
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

    const QRectF source = item->sceneBoundingRect().adjusted(-2, -2, 2, 2);

    const QSize imgSize(480, 360);
    QImage img(imgSize, QImage::Format_ARGB32_Premultiplied);
    const QColor bg(Qt::white);
    img.fill(bg);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, false);
    scene.render(&painter,
                 QRectF(0, 0, imgSize.width(), imgSize.height()),
                 source);
    painter.end();

    const QPointF sceneCenter = item->sceneBoundingRect().center();
    const QPoint center = scenePointToImagePoint(source, imgSize, sceneCenter);

    // 采样区域：缩小，减小边框/抗锯齿影响
    const QRect sample(center.x() - 2, center.y() - 2, 5, 5);
    const QColor avg = averageColor(img, sample);

    // ✅ 鲁棒断言：只要求“明显偏红”，不要求纯红
    QVERIFY2(avg.red() > 100,
             qPrintable(QString("红色分量过低: avg=(%1,%2,%3)").arg(avg.red()).arg(avg.green()).arg(avg.blue())));
    QVERIFY2(avg.red() > avg.green() + 60,
             qPrintable(QString("不是明显偏红: avg=(%1,%2,%3)").arg(avg.red()).arg(avg.green()).arg(avg.blue())));
    QVERIFY2(avg.red() > avg.blue() + 60,
             qPrintable(QString("不是明显偏红: avg=(%1,%2,%3)").arg(avg.red()).arg(avg.green()).arg(avg.blue())));

    scene.removeItem(item);
    delete item;
}

void TestDiagramItemProperties::selectionBorder_state()
{
    QMenu menu;
    DiagramItem *item = new DiagramItem(DiagramItem::Step, &menu, nullptr);
    item->setFixedSize(QSizeF(160, 100));

    QColor fillColor = Qt::yellow;
    item->setBrush(fillColor);

    item->ableEvents();
    item->setSelected(true);
    QVERIFY(item->isSelected());

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

    QCOMPARE(item->m_color, fillColor);
    QCOMPARE(item->boundingRect(), brBefore);
    QCOMPARE(item->getSize(), sizeBefore);

    scene.removeItem(item);
    delete item;
}

void TestDiagramItemProperties::selectionBorder_disableEvents()
{
    QMenu menu;
    DiagramItem *item = new DiagramItem(DiagramItem::Step, &menu, nullptr);
    item->setFixedSize(QSizeF(160, 100));

    QColor fillColor = Qt::cyan;
    item->setBrush(fillColor);

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

    QCOMPARE(item->m_color, fillColor);

    scene.removeItem(item);
    delete item;
}

void TestDiagramItemProperties::size_property_chain()
{
    QMenu menu;
    DiagramItem *item = new DiagramItem(DiagramItem::Step, &menu, nullptr);

    const QSizeF s(250.5, 180.25);
    item->setFixedSize(s);

    QCOMPARE(item->getSize(), s);

    delete item;
}

void TestDiagramItemProperties::text_property_chain()
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

int runDiagramItemPropertiesTests(int argc, char** argv)
{
    TestDiagramItemProperties tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "test_diagramitem_properties.moc"
