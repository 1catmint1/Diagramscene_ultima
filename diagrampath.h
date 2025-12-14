#ifndef DIAGRAMPATH_H
#define DIAGRAMPATH_H

#include<QGraphicsPathItem>
#include<diagramitem.h>
#include<QPainterPath>

class DiagramPath : public QGraphicsPathItem
{
public:
    int start;
    int end;
    int startp;
    int endp;
    enum { Type = UserType +20 };


    DiagramPath(DiagramItem *startItem,DiagramItem *endItem,
                DiagramItem::TransformState startState,
                DiagramItem::TransformState endState,QGraphicsItem *parent=nullptr);

    int type() const override { return Type; }

    void updatePath();
    DiagramItem * getStartItem();
    DiagramItem * getEndItem();

protected:

private:
    DiagramItem *startItem;
    DiagramItem *endItem;
    DiagramItem::TransformState startState;
    DiagramItem::TransformState endState;
    // QPointF startpoint ;
    // QPointF endpoint;

    // QPointF startRectPoint;
    // QPointF endRectPoint;

    QPainterPath m_path;

    int m_quad;
    int m_state;

    void drawHead(QPointF endPoint,QPointF endRectPoint);
    int quad(QPointF startPoint,QPointF endPoint);
    void drawZig(QPointF startPoint,QPointF endPoint);
};

#endif // DIAGRAMPATH_H
