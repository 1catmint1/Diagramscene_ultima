#include "diagrampath.h"
#include<QPainterPath>


DiagramPath::DiagramPath(DiagramItem *startItem,DiagramItem *endItem,
                         DiagramItem::TransformState startState,
                         DiagramItem::TransformState endState,QGraphicsItem *parent) :
    QGraphicsPathItem(parent),startItem(startItem),
    endItem(endItem),startState(startState),endState(endState)
{
    setFlag(QGraphicsItem::ItemIsSelectable,true);
    QPointF startpoint = startItem->mapToScene(startItem->linkWhere()[startState].center());
    QPointF endpoint = endItem->mapToScene(endItem->linkWhere()[endState].center());

    m_quad = quad(startpoint,endpoint);

    m_state = startState*100+endState*10+m_quad;
}

void DiagramPath::updatePath(){

    QPointF startpoint = startItem->mapToScene(startItem->linkWhere()[startState].center());
    QPointF endpoint = endItem->mapToScene(endItem->linkWhere()[endState].center());

    QPointF startRectPoint = startItem->mapToScene(startItem->rectWhere()[startState].center());
    QPointF endRectPoint = endItem->mapToScene(endItem->rectWhere()[endState].center());

    m_quad = quad(startpoint,endpoint);

    m_state = startState*100+endState*10+m_quad;
    // QPainterPath m_path;
    m_path.clear();

    m_path.moveTo(startRectPoint);
    m_path.lineTo(startpoint);
    drawZig(startpoint,endpoint);
    m_path.lineTo(endpoint);
    m_path.lineTo(endRectPoint);


    drawHead(endpoint,endRectPoint);



    // m_path = m_path;
    setPath(m_path);
}

void DiagramPath::drawHead(QPointF endpoint,QPointF endRectPoint){
    if(endpoint.y() == endRectPoint.y()){
        if(endpoint.x() > endRectPoint.x()){
            m_path.lineTo(QPointF(endpoint.x()-5,endpoint.y()-5));
            m_path.moveTo(endRectPoint);
            m_path.lineTo(QPointF(endpoint.x()-5,endpoint.y()+5));
        }
        else if(endpoint.x() < endRectPoint.x()){
            m_path.lineTo(QPointF(endpoint.x()+5,endpoint.y()-5));
            m_path.moveTo(endRectPoint);
            m_path.lineTo(QPointF(endpoint.x()+5,endpoint.y()+5));
        }
    }else if(endpoint.x() == endRectPoint.x()){
        if(endpoint.y() > endRectPoint.y()){
            m_path.lineTo(QPointF(endpoint.x()-5,endpoint.y()-5));
            m_path.moveTo(endRectPoint);
            m_path.lineTo(QPointF(endpoint.x()+5,endpoint.y()-5));
        }
        else if(endpoint.y()<endRectPoint.y()){
            m_path.lineTo(QPointF(endpoint.x()-5,endpoint.y()+5));
            m_path.moveTo(endRectPoint);
            m_path.lineTo(QPointF(endpoint.x()+5,endpoint.y()+5));
        }
    }
}

int DiagramPath::quad(QPointF startPoint, QPointF endPoint)
{
    if(startPoint.x()>=endPoint.x() && startPoint.y()>=endPoint.y()){
        return 4;
    }else if(startPoint.x()<=endPoint.x() && startPoint.y()>=endPoint.y()){
        return 1;
    }else if(startPoint.x()>=endPoint.x() && startPoint.y()<=endPoint.y()){
        return 3;
    }else if(startPoint.x()<=endPoint.x() && startPoint.y()<=endPoint.y()){
        return 2;
    }else{return 0;}
}

void DiagramPath::drawZig(QPointF startPoint,QPointF endPoint)
{
    QPointF midPoint((startPoint.x()+endPoint.x())/2,(startPoint.y()+endPoint.y())/2);
    switch (m_state) {
    case 882:
    case 883:
    case 811:
    case 812:
    case 813:
    case 822:
    case 823:
    case 824:
    case 182:
    case 111:
    case 112:
    case 141:
    case 411:
    case 412:
    case 414:
    case 441:
    case 444:
    case 421:
    case 423:
    case 424:
    case 283:
    case 244:
    case 223:
    case 224:
        m_path.lineTo(QPointF(endPoint.x(),startPoint.y()));
        break;
    case 881:
    case 884:
    case 814:
    case 821:
    case 181:
    case 183:
    case 184:
    case 113:
    case 114:
    case 142:
    case 143:
    case 144:
    case 413:
    case 442:
    case 443:
    case 422:
    case 281:
    case 282:
    case 284:
    case 241:
    case 242:
    case 243:
    case 221:
    case 222:
        m_path.lineTo(startPoint.x(),endPoint.y());
        break;
    case 842:
    case 843:
    case 121:
    case 122:
    case 481:
    case 484:
    case 213:
    case 214:{
        m_path.lineTo(midPoint.x(),startPoint.y());
        m_path.lineTo(midPoint.x(),endPoint.y());
        break;
    }
    case 841:
    case 844:
    case 123:
    case 124:
    case 482:
    case 483:
    case 211:
    case 212:{
        m_path.lineTo(startPoint.x(),midPoint.y());
        m_path.lineTo(endPoint.x(),midPoint.y());
        break;
    }
    default:
        break;
    }
}

DiagramItem* DiagramPath::getStartItem(){
    return startItem;
}

DiagramItem* DiagramPath::getEndItem(){
    return endItem;
}
