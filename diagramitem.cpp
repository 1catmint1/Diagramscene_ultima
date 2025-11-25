// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "diagramitem.h"
#include "arrow.h"
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <diagramtextitem.h>
#include "diagrampath.h"
#include<diagramscene.h>


DiagramItem::DiagramItem(DiagramType diagramType, QMenu *contextMenu, QGraphicsItem *parent)
    : QGraphicsItem(parent),                 // 初始化父类
    myDiagramType(diagramType),            // 初始化图元类型
    m_rotationAngle(0),            // 初始化上下文菜单
    myContextMenu(contextMenu),                           // 初始化边界
    m_border(5),                  // 初始化图元大小
    m_grapSize(150, 100),                     // 初始化最小尺寸
    m_minSize(40, 40)                     // 初始化旋转角度为0度
{
    m_color = Qt::white;

    setFlags(QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges |
             QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true);

    textItem = new DiagramTextItem(this);
    textItem->setPlainText("请输入");  // 设置初始文本
    textItem->setTextInteractionFlags(Qt::TextEditorInteraction);  // 允许文本编辑
    textItem->setPos(boundingRect().center() - QPointF(textItem->boundingRect().width() / 2, textItem->boundingRect().height() / 2));
}

QRectF DiagramItem::boundingRect() const
{

    QRectF rect(QPointF(-20,-20),m_grapSize+QSize(40,40));
    // QRectF rect(QPointF(0, 0), m_grapSize);

    // 使用当前旋转角度计算旋转后的边界框
    QTransform transform;
    transform.rotate(m_rotationAngle);

    return transform.mapRect(rect).normalized();
}

void DiagramItem::setBrush(QColor &color){
    m_color = color;
    update();
}

void DiagramItem::setFixedSize(const QSizeF &size) {
    m_grapSize = size;
    prepareGeometryChange();
    update();
}


void DiagramItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                        QWidget *){

    painter->setRenderHint(QPainter::Antialiasing);

    // 保存当前的绘制状态
    painter->save();

    // 旋转图元，根据当前的旋转角度
    painter->rotate(m_rotationAngle);
    ///////////////////////////////////////////////////////////////////////////
    qreal  w = m_grapSize.width();
    qreal  h = m_grapSize.height();
    qreal  b = m_border;

    QPainterPath path;

    painter->setBrush(m_color);

    switch (myDiagramType) {
    case StartEnd:
        path.moveTo(b+(w-2*b)*0.15, b);
        path.arcTo(QRectF(b,b,(w-2*b)*0.3,h-2*b),90,180);
        path.lineTo(w-b-(w-2*b)*0.15,h-b);
        path.arcTo(QRectF(w-b-(w-2*b)*0.3,b,(w-2*b)*0.3,h-2*b),270,180);
        path.closeSubpath();

        painter->drawPath(path);
        break;

    case Conditional:
        myPolygon.clear();
        myPolygon << QPointF(w/2, m_border) << QPointF(m_border,h/2)
                  << QPointF(w/2, h-m_border) << QPointF(w-m_border, h/2)
                  << QPointF(w/2, m_border);
        painter->drawPolygon(myPolygon);
        break;
    case Step:
        path.addRect(QRectF(QPointF(m_border,m_border),m_grapSize-QSizeF(10,10) ) ); //根据给定尺寸绘制图形
        myPolygon = path.toFillPolygon();
        painter->drawPolygon(myPolygon);
        break;
    case circular:
        path.addEllipse(m_border,m_border,w-2*m_border,h-2*m_border);
        myPolygon = path.toFillPolygon();
        painter->drawPolygon(myPolygon);
        break;
    case Document:{
        // m_grapSize.setWidth(m_grapSize.height() * 1.5);
        // 绘制矩形的左、右和上边
        path.moveTo(QPointF(m_border, m_grapSize.height() - m_border-15));  // 左下角
        path.lineTo(QPointF(m_border, m_border));  // 左上角
        path.lineTo(QPointF(m_grapSize.width() - m_border, m_border));  // 右上角
        path.lineTo(QPointF(m_grapSize.width() - m_border, m_grapSize.height() - m_border-15));  // 右下角


        // 获取矩形的底边角点
        QPointF bottomRight = QPointF(m_grapSize.width() - m_border, m_grapSize.height() - m_border -15);
        QPointF bottomLeft = QPointF(m_border, m_grapSize.height() - m_border -15);

        // 设置波浪线起始点
        qreal waveHeight = 10;
        qreal waveLength = (bottomRight.x() - bottomLeft.x()) / 3;

        // 绘制波浪线的底边，从左下角到右下角
        path.moveTo(bottomLeft);
        path.cubicTo(bottomLeft.x() + waveLength  , bottomLeft.y() + waveHeight + waveHeight/2,
                     bottomLeft.x() + waveLength + waveLength/2, bottomLeft.y() ,
                     bottomRight.x(), bottomRight.y());

        // 绘制矩形的路径
        painter->drawPath(path);
        break;
    }
    case PredefinedProcess:{
        // 设置图形的宽度，确保宽度大于高度
        m_grapSize.setWidth(m_grapSize.height() * 1.5);

        // 绘制矩形的轮廓
        path.addRect(m_border, m_border, m_grapSize.width() - 2 * m_border, m_grapSize.height() - 2 * m_border);

        // 计算内侧两条竖线的位置
        qreal innerLineOffset = m_grapSize.width() / 8;
        QPointF leftLineTop = QPointF(m_border + innerLineOffset, m_border);
        QPointF leftLineBottom = QPointF(m_border + innerLineOffset, m_grapSize.height() - m_border);
        QPointF rightLineTop = QPointF(m_grapSize.width() - m_border - innerLineOffset, m_border);
        QPointF rightLineBottom = QPointF(m_grapSize.width() - m_border - innerLineOffset, m_grapSize.height() - m_border);

        // 绘制左侧内侧竖线
        path.moveTo(leftLineTop);
        path.lineTo(leftLineBottom);

        // 绘制右侧内侧竖线
        path.moveTo(rightLineTop);
        path.lineTo(rightLineBottom);

        // 绘制矩形和内侧线条
        painter->drawPath(path);
        break;
    }

    case StoredData: {
        // 设置图形的宽度，确保宽度大于高度
        // m_grapSize.setWidth(m_grapSize.height() * 1.5);
        path.moveTo(b+(w-2*b)*0.15, b);
        path.arcTo(QRectF(b,b,(w-2*b)*0.3,h-2*b),90,180);
        path.lineTo(w-b-(w-2*b)*0.15,h-b);
        path.moveTo(b+(w-2*b)*0.15, b);
        path.lineTo(w-b-(w-2*b)*0.15,b);
        path.arcTo(QRectF(w-b-(w-2*b)*0.3,b,(w-2*b)*0.3,h-2*b),90,180);
        // path.lineTo(w-b-(w-2*b)*0.15,b);
        // path.closeSubpath();

        painter->drawPath(path);
        break;
    }
    case Memory: {
        // 设置图形的宽度，确保宽度大于高度
        m_grapSize.setWidth(m_grapSize.height() * 1.5);
        // 绘制外部矩形
        path.addRect(m_border, m_border, m_grapSize.width() - 2 * m_border, m_grapSize.height() - 2 * m_border);

        // 计算线条的位置和宽度
        qreal lineWidth = (m_grapSize.height() - 2 * m_border) * 0.1; // 设置线条的宽度为高度的10%
        qreal lineHeight = (m_grapSize.width() - 2 * m_border) * 0.1; // 设置水平线条的高度为宽度的10%

        // 绘制顶部粗线条
        path.moveTo(QPointF(m_border, m_border + lineWidth));
        path.lineTo(QPointF(m_grapSize.width() - m_border, m_border + lineWidth));

        // 绘制左侧粗线条
        path.moveTo(QPointF(m_border + lineHeight, m_border));
        path.lineTo(QPointF(m_border + lineHeight, m_grapSize.height() - m_border));

        // 完成绘制
        painter->drawPath(path);
        break;
    }
    case SequentialAccessStorage: {
        // 设置图形的宽度，确保宽度大于高度
        // m_grapSize.setWidth(m_grapSize.height() * 1.5);
        m_grapSize.setWidth(m_grapSize.height());
        // 定义边距
        qreal b = m_border;

        // 计算圆的直径和中心位置
        qreal diameter = m_grapSize.height() - 2 * b;
        QPointF center(b + diameter / 2, m_grapSize.height() / 2);

        // 绘制大圆
        path.addEllipse(QRectF(center.x() - diameter / 2, center.y() - diameter / 2, diameter, diameter));

        // 绘制底部水平线
        path.moveTo(QPointF(center.x() , m_grapSize.height() - b));
        path.lineTo(QPointF(m_grapSize.width() - b, m_grapSize.height() - b));

        // 绘制路径
        painter->drawPath(path);
        break;
    }
    case Io:
        myPolygon.clear();
        myPolygon << QPointF(b+(w-2*b)*0.2, b) << QPointF(w-b,b)
                  << QPointF(w-b-(w-2*b)*0.2, h-b) << QPointF(b, h-b)
                  << QPointF(b+(w-2*b)*0.2,b);
        painter->drawPolygon(myPolygon);
        break;

    case DirectAccessStorage: {   //多一条竖线
        // 设置图形的宽度，确保宽度大于高度
        m_grapSize.setWidth(m_grapSize.height() * 1.5);
        // 计算圆的直径
        qreal diameter = m_grapSize.height() - 2 * b;

        // 从右上角开始绘制上半部分的直线
        path.moveTo(b+diameter / 2, b);
        // path.lineTo(b + diameter / 2, b);

        // 绘制左侧半圆
        path.arcTo(QRectF(b, b, diameter, diameter), 90, 180);  // 从左上角逆时针绘制半圆

        // 画下半部分的直线连接左侧半圆和右侧整圆的下顶点
        path.lineTo(m_grapSize.width() - diameter / 2, m_grapSize.height() - b);

        // 绘制右侧整圆
        path.arcTo(QRectF(m_grapSize.width() - diameter, b, diameter, diameter), 270, 360);  // 从右上角逆时针绘制整圆
        // path.arcTo(QRectF(m_grapSize.width() - diameter, b, diameter, diameter), 90, 180);
        path.arcTo(QRectF(m_grapSize.width() - diameter, b, diameter, diameter), 270, 180);
        path.closeSubpath();
        // 绘制路径
        painter->drawPath(path);
        break;
    }
    case Disk: {
        // 设置图形的宽度，确保宽度大于高度
        m_grapSize.setWidth(m_grapSize.height());

        // 计算椭圆的长轴和短轴
        qreal ellipseWidth = m_grapSize.width() - 2 * b;
        qreal ellipseHeight = ellipseWidth / 2;  // 椭圆的高为宽的一半

        // 绘制顶部椭圆
        path.moveTo(b, b + ellipseHeight / 2);
        path.arcTo(QRectF(b, b, ellipseWidth, ellipseHeight), 180, 360);

        // 绘制左侧垂直线
        path.lineTo(b, m_grapSize.height() - b - ellipseHeight/2);

        // 绘制底部椭圆
        path.arcTo(QRectF(b, m_grapSize.height() - b - ellipseHeight, ellipseWidth, ellipseHeight), 180, 180);

        // 绘制右侧垂直线
        path.lineTo(b + ellipseWidth, b + ellipseHeight / 2);
        // 绘制顶部椭圆的下半部分
        path.arcTo(QRectF(b, b, ellipseWidth, ellipseHeight), 0, -180);

        // 绘制路径
        // path.closeSubpath();
        painter->drawPath(path);
        break;
    }
    case Card: {
        // 定义宽度和边界
        m_grapSize.setWidth(m_grapSize.height() * 1.5);
        // 设置左下角的起点
        path.moveTo(b, h - b);

        // 绘制左边缘
        path.lineTo(b, b + w * 0.15);

        // 绘制顶部斜边
        path.lineTo(b + w * 0.15, b );

        // 绘制顶部直边
        path.lineTo(w - b, b);

        // 绘制右边缘
        path.lineTo(w - b, h - b);

        // 绘制底部边缘
        path.lineTo(b, h - b);

        // 绘制路径
        painter->drawPath(path);
        break;
    }
    case ManualInput:{
        // 定义宽度和边界
        m_grapSize.setWidth(m_grapSize.height() * 1.5);
        // 设置左下角的起点
        path.moveTo(b, h - b);

        // 绘制左边缘
        path.lineTo(b, b + w * 0.15);

        // 绘制顶部斜边
        path.lineTo(w - b, b );

        // 绘制右边缘
        path.lineTo(w - b, h - b);

        // 绘制底部边缘
        path.lineTo(b, h - b);

        // 绘制路径
        painter->drawPath(path);
        break;
    }
    case PerforatedTape:{    //填充有问题
        m_grapSize.setWidth(m_grapSize.height() * 1.5);

        // 获取矩形的上下边角点
        QPointF topLeft = QPointF(m_border, m_border+10);
        QPointF bottomLeft = QPointF(m_border, m_grapSize.height() - m_border-10);
        QPointF topRight = QPointF(m_grapSize.width() - m_border, m_border+10);
        QPointF bottomRight = QPointF(m_grapSize.width() - m_border, m_grapSize.height() - m_border-10);

        // 设置波浪线参数
        qreal waveHeight = 10;
        qreal waveLength = (bottomRight.x() - bottomLeft.x()) / 3;

        // 1. 从左上角开始绘制左边的竖线
        path.moveTo(topLeft);
        path.lineTo(bottomLeft);

        // 2. 从左下角绘制向右的波浪线，先波谷再波峰
        path.moveTo(bottomLeft);
        path.cubicTo(bottomLeft.x() + waveLength / 2, bottomLeft.y() + 1.5 * waveHeight,
                     bottomLeft.x() + waveLength + waveLength / 2, bottomLeft.y() - 1.5 * waveHeight,
                     bottomRight.x(), bottomRight.y());

        // 3. 从右下角绘制右边的竖线，向上到右上角
        path.lineTo(topRight);

        // 4. 从右上角往左绘制向左的波浪线，先波谷再波峰
        path.moveTo(topRight);
        path.cubicTo(topRight.x() - waveLength / 2, topRight.y() - 1.5 * waveHeight,
                     topRight.x() - waveLength - waveLength / 2, topRight.y() + 1.5 * waveHeight,
                     topLeft.x(), topLeft.y());

        // 绘制路径
        painter->drawPath(path);
        break;
    }
    case Display:{
        // 设置图形的宽度，确保宽度大于高度
        m_grapSize.setWidth(m_grapSize.height() * 1.5);
        // 绘制左上角的斜线
        path.moveTo(b + w * 0.15, b);
        path.lineTo(b, h / 2);  // 第一条斜线
        path.lineTo(b + w * 0.15, h - b);  // 第二条斜线

        // 绘制底部的水平线
        path.lineTo(w - b - (w - 2 * b) * 0.15, h - b);

        // 绘制右侧的半圆
        // path.arcTo(QRectF(w - b - (w - 2 * b) * 0.3, b, (w - 2 * b) * 0.3, h - 2 * b), 90, 180);
        path.arcTo(QRectF(w-b-(w-2*b)*0.3,b,(w-2*b)*0.3,h-2*b),270,180);

        // 绘制顶部的水平线，回到起点
        path.lineTo(b + w * 0.15, b);

        // 关闭路径
        path.closeSubpath();
        painter->drawPath(path);
        break;
    }
    case Preparation: {
        // 设置图形的宽度，确保宽度大于高度
        m_grapSize.setWidth(m_grapSize.height() * 1.5);
        qreal w = m_grapSize.width();
        qreal h = m_grapSize.height();
        qreal b = m_border;

        // 从左上角开始绘制
        path.moveTo(b + w * 0.15, b);  // 左上角的起点
        path.lineTo(b, h / 2);  // 左侧斜边
        path.lineTo(b + w * 0.15, h - b);  // 左侧的第二条斜边

        // 绘制底部水平直线
        path.lineTo(w - b - w * 0.15, h - b);

        // 绘制右侧的第二条斜线
        path.lineTo(w - b, h / 2);

        // 绘制顶部水平直线，回到起点
        path.lineTo(w - b - w * 0.15, b);

        // 关闭路径
        path.closeSubpath();

        // 绘制路径
        painter->drawPath(path);
        break;
    }
    case  ManualOperation: {
        // 设置图形的宽度，确保宽度大于高度
        m_grapSize.setWidth(m_grapSize.height() * 1.5);
        qreal w = m_grapSize.width();
        qreal h = m_grapSize.height();
        qreal b = m_border;

        // 从左下角开始绘制
        path.moveTo(b + w * 0.15, h - b);  // 左下角
        path.lineTo(b , b);  // 左上角，向右移动一定距离
        path.lineTo(w - b , b);  // 右上角，向左移动一定距离
        path.lineTo(w - b - w * 0.15, h - b);  // 右下角
        path.lineTo(b + w * 0.15, h - b);  // 回到左下角

        // 关闭路径
        path.closeSubpath();

        // 绘制路径
        painter->drawPath(path);
        break;
    }
    case ParallelMode: {
        // 定义图形的宽度和高度
        qreal w = m_grapSize.width();
        qreal h = m_grapSize.height();
        qreal b = m_border;

        // 绘制上部直线
        path.moveTo(b, h / 3);
        path.lineTo(w - b, h / 3);

        // 绘制下部直线
        path.moveTo(b, 2 * h / 3);
        path.lineTo(w - b, 2 * h / 3);

        // 绘制路径
        painter->drawPath(path);
        break;
    }
    case Hexagon: {
        // 设置图形的宽度，确保宽度大于高度
        m_grapSize.setWidth(m_grapSize.height() * 1.2); // 宽度稍微大于高度
        qreal w = m_grapSize.width();
        qreal h = m_grapSize.height();
        qreal b = m_border;

        // 从左下角开始绘制
        path.moveTo(b, h - b);  // 左下角
        path.lineTo(b, b + h * 0.15);  // 左上斜边的底部
        path.lineTo(b + w * 0.15, b);  // 左上角的顶点
        path.lineTo(w - b - w * 0.15, b);  // 右上角的顶点
        path.lineTo(w - b, b + h * 0.15);  // 右上斜边的底部
        path.lineTo(w - b, h - b);  // 右下角
        path.lineTo(b, h - b);  // 回到左下角

        // 关闭路径
        path.closeSubpath();

        // 绘制路径
        painter->drawPath(path);
        break;
    }

    default:
        qDebug()<<"this Node does not exist!";
    }

    // 仅更新文本框的位置，不旋转文本框
    textItem->setPos(boundingRect().center() - QPointF(textItem->boundingRect().width() / 2, textItem->boundingRect().height() / 2));

    // 恢复绘制状态
    painter->restore();

    qreal penW = 1.0 / painter->transform().m11();
    QRectF imgRect =
        QRectF(QPointF(m_border, m_border), m_grapSize - QSizeF(10, 10));

    if (isSelected() && isHover && isChange) {
        // 保存画笔状态
        painter->save();

        // 旋转画布与图元同步
        // painter->translate(boundingRect().center());
        painter->rotate(m_rotationAngle);
        // painter->translate(-boundingRect().center());

        QPen borderPen(QColor(0, 120, 215), penW * 2, Qt::DashLine);
        QPen PointPen(QColor(90, 157, 253), penW, Qt::SolidLine);

        // 绘出虚线边框
        painter->setPen(borderPen);
        painter->setBrush(Qt::transparent);
        painter->drawRect(imgRect);

        // 绘出8个角的控制点
        painter->setPen(PointPen);

        const QMap<TransformState, QRectF> rectMap = rectWhere();
        const QMap<TransformState,QRectF> linkMap = linkWhere();
        painter->setBrush(QBrush(Qt::red));
        for (const TransformState state : rectMap.keys()) {
            painter->drawRect(rectMap[state]);
        }
        painter->setBrush(QBrush(Qt::blue));
        for(QRectF link : linkMap){
            painter->drawRect(link);
        }

        // 恢复画笔状态
        painter->restore();
    }

    if(showLink){
        const QMap<TransformState,QRectF> linkMap = linkWhere();
        painter->setBrush(QBrush(Qt::blue));
        for(QRectF link : linkMap){
            painter->drawRect(link);
        }
    }

}


void DiagramItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event) {
    if(!isHover || !isSelected()) return ;


    QGraphicsItem::hoverMoveEvent(event);

    bool isHit = false;
    if(QRectF(QPointF(0,0),m_grapSize).contains(event->pos() ) ){
        const QMap<TransformState, QRectF> rectMap = rectWhere();
        for (const TransformState tsf : rectMap.keys()) {
            if (rectMap[tsf].contains(event->pos())) {
                switch (tsf) {
                case TF_Top:
                case TF_Bottom: {
                    setCursor(QCursor(Qt::SizeVerCursor));
                } break;
                case TF_Left:
                case TF_Right: {
                    setCursor(QCursor(Qt::SizeHorCursor));
                } break;
                case TF_TopL:
                case TF_BottomR: {
                    setCursor(QCursor(Qt::SizeFDiagCursor));
                } break;
                case TF_TopR:
                case TF_BottomL: {
                    setCursor(QCursor(Qt::SizeBDiagCursor));
                } break;
                default: {
                    // setCursor(QCursor(Qt::SizeAllCursor));
                } break;
                }
                m_tfState = tsf;
                isHit = true;
            }
        }
    }
    else{
        const QMap<TransformState,QRectF> linkMap = linkWhere();
        for(const TransformState tsf : linkMap.keys()){
            if (linkMap[tsf].contains(event->pos())) {

                switch (tsf) {
                case TF_Top:
                case TF_Bottom:
                case TF_Left:
                case TF_Right: {
                    setCursor(QCursor(Qt::CrossCursor));
                    isInsertPath = true;
                } break;
                default:
                    break;
                }
                isHit = true;
            }
        }
    }
    if (!isHit) {
        m_tfState = TF_Cen;
        setCursor(QCursor(Qt::ArrowCursor));
        isInsertPath = false;
    }
}

void DiagramItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if(!isChange) return ;
    if (event->buttons() == Qt::LeftButton) {
        // 计算鼠标偏移量
        qreal dx = event->pos().x() - event->lastPos().x();
        qreal dy = event->pos().y() - event->lastPos().y();
        qreal x = pos().x();
        qreal y = pos().y();
        qreal w = m_grapSize.width();
        qreal h = m_grapSize.height();

        // 状态改变，只有允许调整时才会进去相应部分
        uint8_t flgs = uint8_t(m_tfState);
        if ((flgs & 0x01) == 0x01) { // 右
            w = w + dx;
            setFlag(QGraphicsItem::ItemIsMovable, false);
        }
        if ((flgs >> 1 & 0x01) == 0x01) { // 左
            x = x + dx;
            w = w - dx;
            setFlag(QGraphicsItem::ItemIsMovable, false);
        }
        if ((flgs >> 2 & 0x01) == 0x01) { // 下
            h = h + dy;
            setFlag(QGraphicsItem::ItemIsMovable, false);
        }
        if ((flgs >> 3 & 0x01) == 0x01) { // 上
            y = y + dy;
            h = h - dy;
            setFlag(QGraphicsItem::ItemIsMovable, false);
        }

        if (flgs) {
            // 最小宽高像素
            if (w < 40)
                w = 40;
            if (h < 40)
                h = 40;

            // 界面及时做出反馈
            prepareGeometryChange();
            setPos(x, y);
            m_grapSize = QSize(w, h);
        } else {
            setFlag(QGraphicsItem::ItemIsMovable, true);
        }
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void DiagramItem::disableEvents(){
    isHover = false;
    isChange = false;
    setAcceptHoverEvents(false);
}
void DiagramItem::ableEvents(){
    isHover = true;
    isChange = true;
    setAcceptHoverEvents(true);
}

//! [1]
void DiagramItem::removeArrow(Arrow *arrow)
{
    arrows.removeAll(arrow);
}
//! [1]

//! [2]
void DiagramItem::removeArrows()
{
    const auto arrowsCopy = arrows;
    for (Arrow *arrow : arrowsCopy) {
        arrow->startItem()->removeArrow(arrow);
        arrow->endItem()->removeArrow(arrow);
        scene()->removeItem(arrow);
        delete arrow;
    }
}

void DiagramItem::removePath(DiagramPath *path){
    marks.remove(path);
    pathes.removeAll(path);
    qDebug()<<"deleted";
}

void DiagramItem::removePathes(){
    const auto pathesCopy = pathes;
    for(DiagramPath *path : pathesCopy){
        path->getStartItem()->marks.remove(path);
        path->getStartItem()->removePath(path);
        path->getEndItem()->marks.remove(path);
        path->getEndItem()->removePath(path);
        scene()->removeItem(path);
        delete path;
    }
}
//! [2]

//! [3]
void DiagramItem::addArrow(Arrow *arrow)
{
    arrows.append(arrow);
}
//! [3]

//! [4]
QPixmap DiagramItem::image() const
{
    //Step, Conditional, StartEnd, Io,
    // circular,StoredData,Document,PredefinedProcess,
    //     Memory,SequentialAccessStorage,DirectAccessStorage,
    //     Disk,Card,ManualInput
    QPixmap pixmap(250, 250);
    switch (myDiagramType) {
    case Step:
        pixmap.load(":/images/NodesIcon/Step.png");
        break;
    case Conditional:
        pixmap.load(":/images/NodesIcon/Conditional.png");
        break;
    case StartEnd:
        pixmap.load(":/images/NodesIcon/StartEnd.png");
        break;
    case Io:
        pixmap.load(":/images/NodesIcon/Io.png");
        break;
    case circular:
        pixmap.load(":/images/NodesIcon/Circular.png");
        break;
    case StoredData:
        pixmap.load(":/images/NodesIcon/StoredData.png");
        break;
    case Document:
        pixmap.load(":/images/NodesIcon/Document.png");
        break;
    case PredefinedProcess:
        pixmap.load(":/images/NodesIcon/PredefinedProcess.png");
        break;
    case Memory:
        pixmap.load(":/images/NodesIcon/Memory.png");
        break;
    case SequentialAccessStorage:
        pixmap.load(":/images/NodesIcon/SequentialAccessStorage.png");
        break;
    case DirectAccessStorage:
        pixmap.load(":/images/NodesIcon/DirectAccessStorage.png");
        break;
    case Disk:
        pixmap.load(":/images/NodesIcon/Disk.png");
        break;
    case Card:
        pixmap.load(":/images/NodesIcon/Card.png");
        break;
    case ManualInput:
        pixmap.load(":/images/NodesIcon/ManualInput.png");
        break;
    case PerforatedTape:
        pixmap.load(":/images/NodesIcon/PerforatedTape.png");
        break;
    case Display:
        pixmap.load(":/images/NodesIcon/Display.png");
        break;
    case Preparation:
        pixmap.load(":/images/NodesIcon/Preparation.png");
        break;
    case ManualOperation:
        pixmap.load(":/images/NodesIcon/ManualOperation.png");
        break;
    case ParallelMode:
        pixmap.load(":/images/NodesIcon/ParallelMode.png");
        break;
    case Hexagon:
        pixmap.load(":/images/NodesIcon/Hexgon.png");
        break;
    default:
        break;
    }

    return pixmap;
}
//! [4]

//! [5]
void DiagramItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    myContextMenu->popup(event->screenPos());
}
//! [5]

//! [6]
QVariant DiagramItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        for (Arrow *arrow : std::as_const(arrows))
            arrow->updatePosition();
        updatePathes();
    }
    return value;
}
//! [6]
QMap<DiagramItem::TransformState, QRectF> DiagramItem::rectWhere() {
    static QMap<TransformState, QRectF> rectMap;

    int x1 = 0;
    int x2 = m_grapSize.width() / 2 - m_border;
    int x3 = m_grapSize.width() - m_border * 2;
    int y1 = 0;
    int y2 = m_grapSize.height() / 2 - m_border;
    int y3 = m_grapSize.height() - m_border * 2;
    qreal borderWH = m_border * 2 / transform().m11();

    rectMap[TF_TopL] = QRectF(x1, y1, borderWH, borderWH);
    rectMap[TF_Top] = QRectF(x2, y1, borderWH, borderWH);
    rectMap[TF_TopR] = QRectF(x3, y1, borderWH, borderWH);
    rectMap[TF_Right] = QRectF(x3, y2, borderWH, borderWH);
    rectMap[TF_BottomR] = QRectF(x3, y3, borderWH, borderWH);
    rectMap[TF_Bottom] = QRectF(x2, y3, borderWH, borderWH);
    rectMap[TF_BottomL] = QRectF(x1, y3, borderWH, borderWH);
    rectMap[TF_Left] = QRectF(x1, y2, borderWH, borderWH);

    return rectMap;
}

void DiagramItem::setRotationAngle(qreal angle)
{
    // 设置旋转角度
    m_rotationAngle = angle;

    // 重新绘制图元
    update();  // 调用 update() 以重新绘制图元，使其反映新的旋转角度
}

qreal DiagramItem::rotationAngle() const
{
    return m_rotationAngle;
}

void DiagramItem::setSize(QSizeF size){
    m_grapSize = size;
}

void DiagramItem::setWidth(qreal width){
    m_grapSize.setWidth(width);
}

void DiagramItem::setHeight(qreal height){
    m_grapSize.setHeight(height);
}

QSizeF DiagramItem::getSize(){
    return m_grapSize;
}

QMap<DiagramItem::TransformState, QRectF> DiagramItem :: linkWhere(){
    static QMap<TransformState, QRectF> linkMap;

    int x1 = -15;
    int x2 = m_grapSize.width() / 2 - m_border;
    int x3 = m_grapSize.width() - m_border * 2 + 15;
    int y1 = -15;
    int y2 = m_grapSize.height() / 2 - m_border;
    int y3 = m_grapSize.height() - m_border * 2+15;
    qreal borderWH = m_border * 2 / transform().m11();

    linkMap[TF_Top] = QRectF(x2, y1, borderWH, borderWH);
    linkMap[TF_Right] = QRectF(x3, y2, borderWH, borderWH);
    linkMap[TF_Left] = QRectF(x1, y2, borderWH, borderWH);
    linkMap[TF_Bottom] = QRectF(x2, y3, borderWH, borderWH);

    return linkMap;
}

void DiagramItem::addPathes(DiagramPath *path){
    pathes.append(path);
}

void DiagramItem::updatePathes()
{
    for (DiagramPath *path : std::as_const(pathes)){
        path->updatePath();
    }
}
