#ifndef DIAGRAMITEMGROUP_H
#define DIAGRAMITEMGROUP_H

#include<diagramitem.h>

class DiagramItemGroup : public QGraphicsItemGroup
{
public:
    DiagramItemGroup(QGraphicsItem *parent=nullptr);
    QRectF boundingRect() const override;
    void addItem(DiagramItem *item);
    QPointF getTopLeft();
    void addItem(QGraphicsItem *item);
    QList<QPointF> itemScenePos;

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override; //重载悬停函数

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override; //重载移动函数

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *) override; // 重载绘画函数



private:
    qreal top;
    qreal bottom;
    qreal leftBound;
    qreal rightBound;

    QSize m_grapSize;
    QSize m_minSize;
    int m_border;

    QColor m_color;


    enum TransformState {
        TF_Cen = 0x00,
        TF_Top = 0x08,    // 1000
        TF_Bottom = 0x04, // 0100
        TF_Left = 0x02,   // 0010
        TF_Right = 0x01,  // 0001

        TF_TopR = TF_Top | TF_Right,       // 1001
        TF_BottomR = TF_Bottom | TF_Right, // 0101
        TF_BottomL = TF_Bottom | TF_Left,  // 0110
        TF_TopL = TF_Top | TF_Left,        // 1010
    };

    QMap<TransformState, QRectF> rectWhere(); //绘制点
    TransformState m_tfState;


};

#endif // DIAGRAMITEMGROUP_H
