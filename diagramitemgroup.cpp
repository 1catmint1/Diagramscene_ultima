#include "diagramitemgroup.h"
#include<QPen>
#include<QPainter>
#include<QCursor>
#include<QGraphicsSceneHoverEvent>
DiagramItemGroup::DiagramItemGroup(QGraphicsItem *parent) :
    QGraphicsItemGroup(parent),
    top(INFINITY),
    bottom(0),
    leftBound(INFINITY),
    rightBound(0),
    m_minSize(40,40),
    m_border(5)

{
    // for(QGraphicsItem *item : group){
    //     QPointF item_topleft = item->scenePos();
    //     QPointF item_bottomright = item->scenePos()+QPointF(item->boundingRect().width(),item->boundingRect().height());
    //     qDebug()<<item_topleft<<item_bottomright;
    //     if(item_topleft.x() < leftBound) leftBound = item_topleft.x();
    //     if(item_topleft.y() < top) top = item_topleft.y();
    //     if(item_bottomright.x() > rightBound) rightBound = item_bottomright.x();
    //     if(item_bottomright.y() > bottom) bottom = item_bottomright.y();

    // }

    m_grapSize = QSize(rightBound-leftBound,bottom-top);

    setFlags(QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges |
             QGraphicsItem::ItemIsMovable);
    // setHandlesChildEvents(false);
    setAcceptHoverEvents(true);
    qDebug()<<top<<bottom<<leftBound<<rightBound;
}


QRectF DiagramItemGroup::boundingRect() const {
    return QRectF(0,0,m_grapSize.width(),m_grapSize.height());
}

QPointF DiagramItemGroup::getTopLeft() {
    return QPointF(leftBound,top);
}

void DiagramItemGroup::addItem(QGraphicsItem *item){
    QPointF item_topleft = item->scenePos();
    QPointF item_bottomright = item->scenePos()+QPointF(item->boundingRect().width(),item->boundingRect().height());
    qDebug()<<item_topleft<<item_bottomright;
    if(item_topleft.x() < leftBound) leftBound = item_topleft.x();
    if(item_topleft.y() < top) top = item_topleft.y();
    if(item_bottomright.x() > rightBound) rightBound = item_bottomright.x();
    if(item_bottomright.y() > bottom) bottom = item_bottomright.y();
    m_grapSize = QSize(rightBound-leftBound,bottom-top);
    itemScenePos.append(item->scenePos());
    addToGroup(item);
    item->setZValue(this->zValue()-1);
}

void DiagramItemGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                             QWidget *) {
    painter->setRenderHint(QPainter::Antialiasing);
    for(int i=0 ;i<childItems().size();i++){
        childItems().at(i)->setPos(itemScenePos[i]-getTopLeft());
        childItems().at(i)->update();
        // childItems().at(i)->paint(painter,option);
    }
    // this->setSelected(true);


    painter->setBrush(Qt::NoBrush);
    qreal penW = 1.0 / painter->transform().m11();
    QRectF imgRect =
        QRectF(QPointF(m_border, m_border), m_grapSize - QSizeF(10, 10));
    // painter->setPen(QPen(Qt::black));
    // painter->drawRect(imgRect);

    if(isSelected()){

        QPen borderPen(QColor(0, 120, 215), penW * 2, Qt::DashLine);
        QPen PointPen(QColor(90, 157, 253), penW, Qt::SolidLine);

        // 绘出虚线边框
        painter->setPen(borderPen);
        painter->setBrush(Qt::transparent);
        painter->drawRect(imgRect);

        // 绘出8角
        painter->setPen(PointPen);

        const QMap<TransformState, QRectF> rectMap = rectWhere();
        for (const TransformState state : rectMap.keys()) {
            if(state == 0x01 || state == 0x02 || state == 0x04 || state == 0x08){
                painter->setBrush(QColor(0,200,100));
            }else{
                painter->setBrush(QColor(242, 60, 0));
            }
            painter->drawRect(rectMap[state]);
        }
    }
    // QGraphicsItemGroup::paint(painter,option);

}

void DiagramItemGroup::hoverMoveEvent(QGraphicsSceneHoverEvent *event) {
    QGraphicsItem::hoverMoveEvent(event);

    bool isHit = false;
    const QMap<TransformState, QRectF> rectMap = rectWhere();
    for (const TransformState tsf : rectMap.keys()) {
        if (rectMap[tsf].contains(event->pos())) {
            switch (tsf) {
            case TF_Top:
            case TF_Bottom: {
                qDebug()<<"ver";
                setCursor(QCursor(Qt::SizeVerCursor));
            } break;
            case TF_Left:
            case TF_Right: {
                qDebug()<<"Hor";
                setCursor(QCursor(Qt::SizeHorCursor));
            } break;
            case TF_TopL:
            case TF_BottomR: {
                qDebug()<<"FDiag";
                setCursor(QCursor(Qt::SizeFDiagCursor));
            } break;
            case TF_TopR:
            case TF_BottomL: {
                qDebug()<<"BDiag";
                setCursor(QCursor(Qt::SizeBDiagCursor));
            } break;
            default: {
                setCursor(QCursor(Qt::SizeAllCursor));
            } break;
            }
            m_tfState = tsf;
            isHit = true;
        }
    }

    if (!isHit) {
        m_tfState = TF_Cen;
        setCursor(QCursor(Qt::ArrowCursor));
    }
}

void DiagramItemGroup::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (event->buttons() == Qt::LeftButton) {
        // 计算鼠标偏移量
        qreal dx = event->pos().x() - event->lastPos().x();
        qreal dy = event->pos().y() - event->lastPos().y();
        qreal x = pos().x();
        qreal y = pos().y();
        qreal w = m_grapSize.width();
        qreal h = m_grapSize.height();
        QList<QSizeF> childSize;
        QList<QPointF> childPos;
        for(int i=0; i<childItems().size(); i++){
            DiagramItem *item1 = qgraphicsitem_cast<DiagramItem *>(childItems().at(i));
            childSize.append(item1->getSize());
            childPos.append(item1->pos());
        }

        // 状态改变，只有允许调整时才会进去相应部分
        uint8_t flgs = uint8_t(m_tfState);
        if ((flgs & 0x01) == 0x01) { // 右
            w = w + dx;
            for(int i=0; i<childItems().size(); i++){
                childSize[i] = QSizeF(childSize[i].width()+dx,childSize[i].height());
                childItems()[i]->setFlag(QGraphicsItem::ItemIsMovable, false);
            }
            setFlag(QGraphicsItem::ItemIsMovable, false);
        }
        if ((flgs >> 1 & 0x01) == 0x01) { // 左
            x = x + dx;
            w = w - dx;
            for(int i=0; i<childItems().size(); i++){
                childPos[i] = QPointF(childPos[i].x() + dx,childPos[i].y());
                childSize[i] = QSizeF(childSize[i].width()-dx,childSize[i].height());
                childItems()[i]->setFlag(QGraphicsItem::ItemIsMovable, false);
            }
            setFlag(QGraphicsItem::ItemIsMovable, false);
        }
        if ((flgs >> 2 & 0x01) == 0x01) { // 下
            h = h + dy;
            for(int i=0; i<childItems().size(); i++){
                childSize[i] = QSizeF(childSize[i].width(),childSize[i].height() + dy);
                childItems()[i]->setFlag(QGraphicsItem::ItemIsMovable, false);
            }
            setFlag(QGraphicsItem::ItemIsMovable, false);
        }
        if ((flgs >> 3 & 0x01) == 0x01) { // 上
            y = y + dy;
            h = h - dy;
            for(int i=0; i<childItems().size(); i++){
                childPos[i] = QPointF(childPos[i].x(),childPos[i].y() + dy);
                childSize[i] = QSizeF(childSize[i].width(),childSize[i].height() - dy);
                childItems()[i]->setFlag(QGraphicsItem::ItemIsMovable, false);
            }
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
            for(int i=0; i<childItems().size(); i++){
                DiagramItem *item1 = qgraphicsitem_cast<DiagramItem *>(childItems().at(i));
                item1->setPos(childPos[i]);
                item1->setSize(childSize[i]);
            }

        } else {
            setFlag(QGraphicsItem::ItemIsMovable, true);
            for(int i=0; i<childItems().size(); i++){
                DiagramItem *item1 = qgraphicsitem_cast<DiagramItem *>(childItems().at(i));
                item1->setSize(childSize[i]);
                item1->setPos(childPos[i]);
            }
        }
    }
    QGraphicsItem::mouseMoveEvent(event);
}


QMap<DiagramItemGroup::TransformState, QRectF> DiagramItemGroup::rectWhere() {
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


