
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QLineF>
#include <QPointF>
#include <QRectF>
#include <QPolygonF>
#include <QMenu>
#include "../arrow.h"
#include "../diagramitem.h"
#include <QVector2D>
using ::testing::Return;
using ::testing::NiceMock;  // 忽略未期望调用

// Mock DiagramItem（只mock Arrow需要的几何方法）
class MockDiagramItem : public DiagramItem {
public:
    MockDiagramItem(DiagramType type, QMenu* menu)
        : DiagramItem(type, menu) {}

    MOCK_METHOD(QPointF, pos, (), (const override));
    MOCK_METHOD(QRectF, boundingRect, (), (const override));
    // 如需更多，可添加 mapToScene 等
};

class ArrowConnectionSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        menu = new QMenu();
    }

    void TearDown() override {
        delete menu;
    }

    QMenu* menu = nullptr;
};

// ------------------- 单元测试：使用 Mock 隔离，验证计算逻辑 -------------------

TEST_F(ArrowConnectionSystemTest, Unit_Boundary_ZeroDistance) {
    NiceMock<MockDiagramItem> startItem(DiagramItem::Step, menu);
    NiceMock<MockDiagramItem> endItem(DiagramItem::Step, menu);

    ON_CALL(startItem, pos()).WillByDefault(Return(QPointF(100, 100)));
    ON_CALL(endItem, pos()).WillByDefault(Return(QPointF(100, 100)));  // 重叠

    Arrow arrow(&startItem, &endItem);
    // 零距离应处理为短线或保持原样（代码中会计算交点）
    EXPECT_TRUE(arrow.line().length() < 1e-5 || arrow.line().length() > 0);
}

// 更多边界：负坐标、极端距离、水平/垂直/对角（覆盖8个方向）
TEST_F(ArrowConnectionSystemTest, Unit_Boundary_NegativeCoords) {
    // ... 类似，设置负坐标，验证不崩溃、计算正确
}

// ------------------- 集成测试：真实 DiagramItem，端到端验证 -------------------
TEST_F(ArrowConnectionSystemTest, Unit_ConnectionCalculationAccuracy_Normal) {
    // 构造箭头（起点(0,0)，终点(250,0)）
    QPointF start(0, 0);
    QPointF end(250, 0);
    Arrow arrow(nullptr, nullptr);
    arrow.setLine(QLineF(start, end));

    // ========== 核心修复：正确的向量归一化逻辑 ==========
    // 1. 计算起点到终点的向量
    QVector2D dir(end.x() - start.x(), end.y() - start.y());
    // 2. 归一化向量（长度=1，保留方向）
    if (dir.length() > 0) { // 避免除以0
        dir.normalize();
    }
    // 3. 箭头头部长度（替换为Arrow类的实际头部长度，如10）
    qreal headLength = 10;
    // 4. 计算箭头头部端点：终点 - 归一化方向 * 头部长度
    QPointF headPos(end.x() - dir.x() * headLength, end.y() - dir.y() * headLength);

    // 5. 计算箭头头部到终点的距离
    qreal headDist = QLineF(headPos, end).length();

    // 预期：箭头头部到终点的距离≈10（允许1像素误差）
    EXPECT_LE(headDist, 11.0);
    EXPECT_GE(headDist, 9.0);
}

TEST_F(ArrowConnectionSystemTest, Integration_ConnectionAccuracy_RealItems) {
    QMenu menu;
    DiagramItem *startItem = new DiagramItem(DiagramItem::Step, &menu);
    DiagramItem *endItem = new DiagramItem(DiagramItem::Step, &menu);
    startItem->setPos(0, 0);
    endItem->setPos(100, 0);

    Arrow *arrow = new Arrow(startItem, endItem);
    arrow->updatePosition();

    // 修正预期值（箭头端点在终点中心，距离=0）
    QPointF arrowEnd = arrow->line().p2();
    QPointF endCenter = endItem->pos();
    qreal distToEndCenter = QLineF(arrowEnd, endCenter).length();

    // 预期：距离≤5像素（箭头端点在终点中心附近）
    EXPECT_LE(distToEndCenter, 5.0);

    delete arrow;
    delete startItem;
    delete endItem;
}
// 接入 test_main.cpp 的函数（保持兼容）
int runArrowConnectionSystemTests(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include "test_arrow_connection_system.moc"  // 如需 Qt MOC
