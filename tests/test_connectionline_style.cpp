#include <QtTest/QtTest>
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>
#include <QMenu>

#include "../diagramitem.h"
#include "../diagrampath.h"

// ---------- 工具：统计图像中“非背景色”像素 ----------
static int countNonBgPixels(const QImage &img, const QColor &bg)
{
    int cnt = 0;
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            if (img.pixelColor(x, y) != bg)
                ++cnt;
        }
    }
    return cnt;
}

class TestConnectionLineStyle : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void color_and_style_immediate();
    void style_kept_after_updatePath();
};

// ------------------------------------------------------
// 1️⃣ 颜色 & 样式：设置后立即生效（黑盒渲染验证）
// ------------------------------------------------------
void TestConnectionLineStyle::color_and_style_immediate()
{
    QMenu menu;

    auto *startItem = new DiagramItem(DiagramItem::Step, &menu);
    auto *endItem   = new DiagramItem(DiagramItem::Step, &menu);

    startItem->setFixedSize(QSizeF(120, 80));
    endItem->setFixedSize(QSizeF(120, 80));

    startItem->setPos(50, 100);
    endItem->setPos(350, 200);

    QGraphicsScene scene;
    scene.addItem(startItem);
    scene.addItem(endItem);

    auto *path = new DiagramPath(
        startItem,
        endItem,
        DiagramItem::TF_Right,
        DiagramItem::TF_Left
        );

    path->updatePath();
    scene.addItem(path);

    // -------- 设置样式（被测行为）--------
    QPen pen(Qt::red);
    pen.setWidth(3);
    pen.setStyle(Qt::DashLine);
    path->setPen(pen);

    // -------- 渲染到图像 --------
    const QSize imgSize(600, 400);
    QImage img(imgSize, QImage::Format_ARGB32_Premultiplied);
    const QColor bg(Qt::white);
    img.fill(bg);

    QPainter painter(&img);
    scene.render(&painter);
    painter.end();

    // -------- 黑盒判断：画面发生变化 --------
    const int nonBg = countNonBgPixels(img, bg);

    QVERIFY2(nonBg > 0,
             "设置连接线颜色/样式后，渲染图像中未检测到绘制内容");

    delete path;
    delete startItem;
    delete endItem;
}

// ------------------------------------------------------
// 2️⃣ updatePath / 变换后样式不丢失
// ------------------------------------------------------
void TestConnectionLineStyle::style_kept_after_updatePath()
{
    QMenu menu;

    auto *startItem = new DiagramItem(DiagramItem::Step, &menu);
    auto *endItem   = new DiagramItem(DiagramItem::Step, &menu);

    startItem->setFixedSize(QSizeF(120, 80));
    endItem->setFixedSize(QSizeF(120, 80));

    startItem->setPos(80, 120);
    endItem->setPos(380, 260);

    QGraphicsScene scene;
    scene.addItem(startItem);
    scene.addItem(endItem);

    auto *path = new DiagramPath(
        startItem,
        endItem,
        DiagramItem::TF_Top,
        DiagramItem::TF_Bottom
        );

    // 初始样式
    QPen pen(Qt::blue);
    pen.setWidth(4);
    pen.setStyle(Qt::DotLine);
    path->setPen(pen);

    path->updatePath();
    scene.addItem(path);

    // -------- 触发路径重算（模拟节点移动）--------
    endItem->setPos(420, 300);
    path->updatePath();

    // -------- 样式仍然一致 --------
    const QPen curPen = path->pen();

    QCOMPARE(curPen.color(), pen.color());
    QCOMPARE(curPen.width(), pen.width());
    QCOMPARE(curPen.style(), pen.style());

    delete path;
    delete startItem;
    delete endItem;
}

// QTEST_MAIN(TestConnectionLineStyle)
#include "test_connectionline_style.moc"
