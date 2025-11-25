// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "diagramscene.h"
#include "arrow.h"
#include "diagramitem.h"
#include "qaction.h"
#include "diagrampath.h"

#include <QGraphicsSceneMouseEvent>
#include <QTextCursor>
#include <QPainter>
#include <QTimer>

//! [0]
bool isInsertPath = false;

DiagramScene::DiagramScene(QMenu *itemMenu, QObject *parent)
    : QGraphicsScene(parent)
{
    myItemMenu = itemMenu;
    myMode = MoveItem;
    myItemType = DiagramItem::Step;
    line = nullptr;
    textItem = nullptr;
    myItemColor = Qt::white;
    myTextColor = Qt::black;
    myLineColor = Qt::black;
}
//! [0]
//! [1]
void DiagramScene::setLineColor(const QColor &color)
{
    myLineColor = color;
    if (isItemChange(Arrow::Type)) {
        Arrow *item = qgraphicsitem_cast<Arrow *>(selectedItems().first());
        item->setColor(myLineColor);
        update();
    }
}
//! [1]

//! [2]
void DiagramScene::setTextColor(const QColor &color)
{
    myTextColor = color;
    if (isItemChange(DiagramTextItem::Type)) {
        DiagramTextItem *item = qgraphicsitem_cast<DiagramTextItem *>(selectedItems().first());
        item->setDefaultTextColor(myTextColor);
        item->text_color = myTextColor;
    }
}
//! [2]

//! [3]
void DiagramScene::setItemColor(const QColor &color)
{
    myItemColor = color;
    foreach (QGraphicsItem *item, selectedItems()) {
        // 由于items()只返回顶级项，我们不需要担心重复遍历子项
        // qDebug()<<item->mapToScene(item->boundingRect().center()).rx();
        if (isItemChange(DiagramItem::Type)) {
            DiagramItem *item1 = qgraphicsitem_cast<DiagramItem *>(item);
            if(item1!=nullptr){
                item1->setBrush(myItemColor);
            }
        }
    }
}
//! [3]

//! [4]
void DiagramScene::setFont(const QFont &font)
{
    myFont = font;

    if (isItemChange(DiagramTextItem::Type)) {
        QGraphicsTextItem *item = qgraphicsitem_cast<DiagramTextItem *>(selectedItems().first());
        //At this point the selection can change so the first selected item might not be a DiagramTextItem
        if (item)
            item->setFont(myFont);
    }
}
//! [4]

void DiagramScene::setMode(Mode mode)
{
    myMode = mode;
}

void DiagramScene::setItemType(DiagramItem::DiagramType type)
{
    myItemType = type;
}

//! [5]
void DiagramScene::editorLostFocus(DiagramTextItem *item)
{
    QTextCursor cursor = item->textCursor();
    cursor.clearSelection();
    item->setTextCursor(cursor);

    if (item->toPlainText().isEmpty()) {
        removeItem(item);
        item->deleteLater();
    }
}
//! [5]
void DiagramScene::keyPressEvent(QKeyEvent *event)
{
    // 检查是否有选中的图元
    if (!selectedItems().isEmpty()) {
        // 获取当前选中的第一个图元
        QGraphicsItem *item = selectedItems().first();

        // 检查选中的图元是否是 DiagramItem 类型
        DiagramItem *diagramItem = dynamic_cast<DiagramItem *>(item);
        if (diagramItem) {
            switch (event->key()) {
            case Qt::Key_R:  // 按下 R 键，右旋
                diagramItem->setRotationAngle(diagramItem->rotationAngle() + 5);  // 每次旋转5度
                break;
            case Qt::Key_L:  // 按下 L 键，左旋
                diagramItem->setRotationAngle(diagramItem->rotationAngle() - 5);  // 每次旋转5度
                break;
            default:
                QGraphicsScene::keyPressEvent(event);  // 其他按键保持默认处理
            }
        }
    } else {
        // 没有选中的图元，调用基类的默认键盘处理
        QGraphicsScene::keyPressEvent(event);
    }
}
//! [6]
void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() != Qt::LeftButton)
        return;

    if(isInsertPath){
        premode = myMode;
        this->setMode(InsertPath);
        qDebug()<<"yes";
    }

    DiagramItem *item;
    switch (myMode) {
    case InsertItem:
        item = new DiagramItem(myItemType, myItemMenu);
        item->setBrush(myItemColor);
        addItem(item);
        item->setPos(mouseEvent->scenePos());
        emit itemInserted(item);
        break;

    case InsertLine:
        line = new QGraphicsLineItem(QLineF(mouseEvent->scenePos(),
                                            mouseEvent->scenePos()));
        line->setPen(QPen(myLineColor, 2));
        addItem(line);
        break;

    case InsertText:
        textItem = new DiagramTextItem();
        textItem->setFont(myFont);
        textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
        textItem->setZValue(1000.0);
        connect(textItem, &DiagramTextItem::lostFocus,
                this, &DiagramScene::editorLostFocus);
        connect(textItem, &DiagramTextItem::selectedChange,
                this, &DiagramScene::itemSelected);
        addItem(textItem);
        textItem->setDefaultTextColor(myTextColor);
        textItem->setPos(mouseEvent->scenePos());
        emit textInserted(textItem);
        break;

    case MoveItem:
        if (mouseEvent->modifiers() == Qt::ShiftModifier) {
            // Shift 键被按下，进入框选模式
            beginpoint = mouseEvent->scenePos();
            ischeckingbox = true;
        } else {
            // 正常的图元移动逻辑
            movedItem = itemAt(mouseEvent->scenePos(), QTransform());  // 获取当前鼠标下的图元
            if (movedItem != nullptr) {
                // 确保 movedItem 是 DiagramItem 类型，以便访问 isMoving
                DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem*>(movedItem);
                if (diagramItem != nullptr) {
                    diagramItem->isMoving = true;  // 设置为正在移动
                }
            }
        }
        break;
    case InsertPath:{
        setLinkVisible(true);
        pathLine = new QGraphicsLineItem(QLineF(mouseEvent->scenePos(),mouseEvent->scenePos()));
        QPen *pen = new QPen(Qt::blue);
        pen->setStyle(Qt::DashLine);
        pathLine->setPen(*pen);
        addItem(pathLine);}
    default:
        break;
    }

    // 传递事件给父类进行处理
    QGraphicsScene::mousePressEvent(mouseEvent);
}
//! [9]

//! [10]
void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (myMode == InsertLine && line != nullptr) {
        QLineF newLine(line->line().p1(), mouseEvent->scenePos());
        line->setLine(newLine);
    } else if (myMode == MoveItem) {
        if (mouseEvent->modifiers() == Qt::ShiftModifier && ischeckingbox) {
            // 框选逻辑，保持不变
            endpoint = mouseEvent->scenePos();
            qreal width = endpoint.rx() - beginpoint.rx();
            qreal height = endpoint.ry() - beginpoint.ry();

            if (!rectItem) {
                QColor semiTransparentBlue(Qt::blue);
                semiTransparentBlue.setAlpha(50);
                rectItem = addRect(QRectF(beginpoint, QSizeF(width, height)), QPen(Qt::black, 1, Qt::DashLine), QBrush(semiTransparentBlue));
            } else {
                rectItem->setRect(QRectF(beginpoint, QSizeF(width, height)));
            }
            update();
        }
        else {
            // 拖拽图元的逻辑
            if (movedItem && mouseEvent->buttons() & Qt::LeftButton) {
                bool needAlignX = false;
                bool needAlignY = false;
                bool needAlignRight = false;   // 新增：右边界对齐标志
                bool needAlignBottom = false;  // 新增：底边界对齐标志
                bool needAlignCenterX = false;
                bool needAlignCenterY = false;

                // 清除对齐状态
                isleft = false;
                isright = false;    // 清除右边界对齐状态
                istop = false;
                isbottom = false;   // 清除底边界对齐状态
                iscenterX = false;  // 清除 X 方向中心对齐状态
                iscenterY = false;  // 清除 Y 方向中心对齐状态
                alignedItem = nullptr;

                QPointF newPosition = movedItem->pos();
                QPointF suggestedPosition = movedItem->pos();  // 用来保存潜在的吸附位置，但不立即应用

                // 遍历场景中的所有图元
                foreach (QGraphicsItem *item, items()) {
                    // 类型检查和排除当前正在移动的图元
                    DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem*>(item);
                    DiagramItem *diagramMovedItem = qgraphicsitem_cast<DiagramItem*>(movedItem);

                    if (diagramItem != nullptr && diagramMovedItem != nullptr && diagramItem != diagramMovedItem) {
                        QRectF movedRect = diagramMovedItem->sceneBoundingRect();
                        QRectF itemRect = diagramItem->sceneBoundingRect();

                        // 计算左边界的水平距离
                        qreal x_distance = movedRect.left() - itemRect.left();
                        // 计算顶边界的垂直距离
                        qreal y_distance = movedRect.top() - itemRect.top();
                        // 计算右边界的水平距离
                        qreal right_distance = movedRect.right() - itemRect.right();
                        // 计算底边界的垂直距离
                        qreal bottom_distance = movedRect.bottom() - itemRect.bottom();
                        // 计算中心对齐的水平距离（X 方向）
                        qreal center_x_distance = movedRect.center().x() - itemRect.center().x();
                        // 计算中心对齐的垂直距离（Y 方向）
                        qreal center_y_distance = movedRect.center().y() - itemRect.center().y();

                        // 检测水平对齐
                        if (qAbs(x_distance) < 50) {
                            isleft = true;  // 设置水平对齐标志
                            alignedItem = item;
                            // newPosition.setX(itemRect.left());  // 吸附到 x 轴
                            suggestedPosition.setX(itemRect.left());  // 保存潜在的X坐标
                            needAlignX = true;
                        }

                        // 检测垂直对齐
                        if (qAbs(y_distance) < 50) {
                            istop = true;   // 设置垂直对齐标志
                            alignedItem = item;
                            // newPosition.setY(itemRect.top());  // 吸附到 y 轴
                            suggestedPosition.setY(itemRect.top());  // 保存潜在的Y坐标
                            needAlignY = true;
                        }

                        // 检测右边界的水平对齐
                        if (qAbs(right_distance) < 50) {
                            isright = true;  // 新增：设置右边界对齐标志
                            alignedItem = item;
                            suggestedPosition.setX(itemRect.right() - movedRect.width());  // 吸附到右边界
                            needAlignRight = true;
                        }

                        // 检测底边界的垂直对齐
                        if (qAbs(bottom_distance) < 50) {
                            isbottom = true;  // 新增：设置底边界对齐标志
                            alignedItem = item;
                            suggestedPosition.setY(itemRect.bottom() - movedRect.height());  // 吸附到底边界
                            needAlignBottom = true;
                        }

                        // 检测中心对齐（X 方向）
                        if (qAbs(center_x_distance) < 50) {
                            iscenterX = true;  // 设置中心对齐标志
                            alignedItem = item;
                            // newPosition.setX(itemRect.center().x() - movedRect.width() / 2);  // 中心对齐吸附
                            suggestedPosition.setX(itemRect.center().x() - movedRect.width() / 2 + 20);  // 保存潜在的中心对齐位置
                            needAlignCenterX = true;
                        }
                        // 检测中心对齐（Y 方向）
                        if (qAbs(center_y_distance) < 50) {
                            iscenterY = true;  // 新增：设置中心对齐标志（Y 方向）
                            alignedItem = item;
                            suggestedPosition.setY(itemRect.center().y() - movedRect.height() / 2 + 20);  // 保存潜在的中心对齐位置（Y）
                            needAlignCenterY = true;
                        }
                    }
                }

                // 统一更新位置
                if (needAlignX || needAlignY || needAlignRight || needAlignBottom || needAlignCenterX || needAlignCenterY) {
                    // movedItem->setPos(newPosition);
                    alignPosition = suggestedPosition;  // 保存潜在的对齐位置，待 release 时处理
                    update();  // 更新辅助线的绘制
                }
            }
        }
        // 保留原有的鼠标移动事件逻辑
        QGraphicsScene::mouseMoveEvent(mouseEvent);
    }else if(myMode == InsertPath && pathLine != nullptr){
        QLineF newLine(pathLine->line().p1(), mouseEvent->scenePos());
        pathLine->setLine(newLine);
    }
}
//! [10]
void DiagramScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    QPen pen;
    pen.setStyle(Qt::DashLine);  // 使用虚线绘制辅助线
    painter->setPen(pen);

    // 输出对齐辅助线的相关信息
    if (movedItem != nullptr && alignedItem != nullptr) {
        // 打印当前正在拖拽的图元的坐标
        QRectF movedRect = movedItem->sceneBoundingRect();
        // qDebug() << "正在拖拽的图元左边界: " << movedRect.left()
        //          << ", 顶边: " << movedRect.top()
        //          << ", 中心 X 坐标: " << movedRect.center().x()
        //          << ", 宽度: " << movedRect.width()
        //          << ", 高度: " << movedRect.height();

        QRectF itemRect = alignedItem->sceneBoundingRect();  // 获取 alignedItem 的边界
        // qDebug() << "对齐图元的左边界: " << itemRect.left()
        //          << ", 顶边: " << itemRect.top()
        //          << ", 中心 X 坐标: " << itemRect.center().x();

        // qDebug() << "场景矩形的顶部: " << rect.top()
        //          << ", 底部: " << rect.bottom()
        //          << ", 左边: " << rect.left()
        //          << ", 右边: " << rect.right();

        // 如果需要绘制左边界垂直辅助线
        if (isleft) {
            // qDebug() << "正在绘制左边界垂直辅助线，x 位置: " << itemRect.left();
            qreal topY = qMin(itemRect.top(), movedRect.top());  // 辅助线的顶部坐标
            qreal bottomY = qMax(itemRect.bottom(), movedRect.bottom());  // 辅助线的底部坐标
            painter->drawLine(itemRect.left()+6, topY - 20, itemRect.left()+6, bottomY + 20);
        }
        // 如果需要绘制右边界的垂直辅助线
        if (isright) {
            // qDebug() << "正在绘制右边界垂直辅助线，x 位置: " << itemRect.right();
            // 辅助线长度为 alignedItem 和 movedItem 的 y 范围
            qreal topY = qMin(itemRect.top(), movedRect.top());  // 辅助线的顶部坐标
            qreal bottomY = qMax(itemRect.bottom(), movedRect.bottom());  // 辅助线的底部坐标
            painter->drawLine(itemRect.right()-6, topY - 20, itemRect.right()-6, bottomY + 20);  // 绘制右边界辅助线
        }
        // 如果需要绘制顶边界的水平辅助线
        if (istop) {
            // qDebug() << "正在绘制顶边界垂直辅助线，y 位置: " << itemRect.top();
            qreal leftX = qMin(itemRect.left(), movedRect.left());  // 辅助线的左侧坐标
            qreal rightX = qMax(itemRect.right(), movedRect.right());  // 辅助线的右侧坐标
            painter->drawLine(leftX - 20, itemRect.top()+6, rightX + 20, itemRect.top()+6);
        }
        // 如果需要绘制底边界的水平辅助线
        if (isbottom) {
            // qDebug() << "正在绘制底边界水平辅助线，y 位置: " << itemRect.bottom();
            // 辅助线长度为 alignedItem 和 movedItem 的 x 范围
            qreal leftX = qMin(itemRect.left(), movedRect.left());  // 辅助线的左侧坐标
            qreal rightX = qMax(itemRect.right(), movedRect.right());  // 辅助线的右侧坐标
            painter->drawLine(leftX- 20, itemRect.bottom()-6, rightX+ 20, itemRect.bottom()-6);  // 绘制底边界辅助线
        }
        // 如果需要绘制中心对齐的垂直辅助线（X 方向中心对齐）
        if (iscenterX) {
            // qDebug() << "正在绘制中心对齐的垂直辅助线，中心 X 坐标: " << itemRect.center().x();
            qreal topY = qMin(itemRect.top(), movedRect.top());  // 辅助线的顶部坐标
            qreal bottomY = qMax(itemRect.bottom(), movedRect.bottom());  // 辅助线的底部坐标
            painter->drawLine(itemRect.center().x(), topY - 20, itemRect.center().x(), bottomY + 20);
        }

        // 如果需要绘制中心对齐的水平辅助线（Y 方向中心对齐）
        if (iscenterY) {
            // qDebug() << "正在绘制中心对齐的水平辅助线，中心 Y 坐标: " << itemRect.center().y();
            qreal leftX = qMin(itemRect.left(), movedRect.left());  // 辅助线的左侧坐标
            qreal rightX = qMax(itemRect.right(), movedRect.right());  // 辅助线的右侧坐标
            painter->drawLine(leftX - 20, itemRect.center().y(), rightX + 20, itemRect.center().y());
        }
    }
}

//! [11]
void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (line != nullptr && myMode == InsertLine) {
        QList<QGraphicsItem *> startItems = items(line->line().p1());
        if (startItems.count() && startItems.first() == line)
            startItems.removeFirst();
        QList<QGraphicsItem *> endItems = items(line->line().p2());
        if (endItems.count() && endItems.first() == line)
            endItems.removeFirst();

        removeItem(line);
        delete line;

        if (startItems.count() > 0 && endItems.count() > 0 &&
            startItems.first()->type() == DiagramItem::Type &&
            endItems.first()->type() == DiagramItem::Type &&
            startItems.first() != endItems.first()) {
            DiagramItem *startItem = qgraphicsitem_cast<DiagramItem *>(startItems.first());
            DiagramItem *endItem = qgraphicsitem_cast<DiagramItem *>(endItems.first());
            Arrow *arrow = new Arrow(startItem, endItem);
            arrow->setColor(myLineColor);
            startItem->addArrow(arrow);
            endItem->addArrow(arrow);
            arrow->setZValue(-1000.0);
            addItem(arrow);
            arrow->updatePosition();
        }
    }
    else if (ischeckingbox) {
        endpoint = mouseEvent->scenePos();
        removeItem(rectItem);
        rectItem = nullptr;

        int x1 = beginpoint.rx() > endpoint.rx() ? endpoint.rx() : beginpoint.rx();
        int x2 = beginpoint.rx() < endpoint.rx() ? endpoint.rx() : beginpoint.rx();
        int y1 = beginpoint.ry() > endpoint.ry() ? endpoint.ry() : beginpoint.ry();
        int y2 = beginpoint.ry() < endpoint.ry() ? endpoint.ry() : beginpoint.ry();

        foreach (QGraphicsItem *item, this->items()) {
            if (item->mapToScene(item->boundingRect().center()).rx() < x2 &&
                item->mapToScene(item->boundingRect().center()).rx() > x1 &&
                item->mapToScene(item->boundingRect().center()).ry() > y1 &&
                item->mapToScene(item->boundingRect().center()).ry() < y2) {
                item->setSelected(true);
            }
        }
        ischeckingbox = false;
        update();
    }
    else if (myMode == MoveItem) {
        // 拖拽结束后，进行吸附
        if (movedItem != nullptr) {
            // 检查是否需要对齐
            if (isleft || isright || istop || isbottom || iscenterX || iscenterY)  {
                // 如果有对齐需求，则使用在 mouseMoveEvent 中计算的 alignPosition
                movedItem->setPos(alignPosition);  // 应用保存的吸附位置
            }

            // 将拖动图元的 isMoving 设置为 false
            DiagramItem *diagramItem = qgraphicsitem_cast<DiagramItem*>(movedItem);
            if (diagramItem != nullptr) {
                diagramItem->isMoving = false;  // 重置 isMoving 状态
                diagramItem->updatePathes();
            }

            // 清除对齐状态
            isleft = false;
            isright = false;    // 清除右边界对齐状态
            istop = false;
            isbottom = false;   // 清除底边界对齐状态
            iscenterX = false;  // 清除 X 方向中心对齐状态
            iscenterY = false;  // 清除 Y 方向中心对齐状态
            alignedItem = nullptr;


            QTimer::singleShot(300, this, [this]() {
                update();  // 清除辅助线（延迟0.3秒）
            });

            // 最后清除移动图元
            movedItem = nullptr;  // 清除移动图元
        }

        update();  // 清除辅助线（延迟0.3秒）

    }else if(myMode == InsertPath){

        QList<QGraphicsItem *> startItems = items(pathLine->line().p1());
        if (startItems.count() && startItems.first() == pathLine)
            startItems.removeFirst();
        QList<QGraphicsItem *> endItems = items(pathLine->line().p2());
        if (endItems.count() && endItems.first() == pathLine)
            endItems.removeFirst();


        if (startItems.count() > 0 && endItems.count() > 0 &&
            startItems.first()->type() == DiagramItem::Type &&
            endItems.first()->type() == DiagramItem::Type &&
            startItems.first() != endItems.first()) {
            DiagramItem *startItem = qgraphicsitem_cast<DiagramItem *>(startItems.first());
            DiagramItem *endItem = qgraphicsitem_cast<DiagramItem *>(endItems.first());



            DiagramItem::TransformState startState = DiagramItem::TF_Cen;
            DiagramItem::TransformState endState = DiagramItem::TF_Cen;

            QRectF tempRect;


            for(DiagramItem::TransformState state : startItem->linkWhere().keys()){
                tempRect = startItem->mapRectToScene(startItem->linkWhere()[state]);
                QRectF *magnetRect = new QRectF(tempRect.topLeft()-QPointF(20,20),tempRect.size()+QSize(40,40));
                if(  magnetRect->contains(pathLine->line().p1()) ) {

                    startState = state;
                    qDebug()<<startState;
                    break;
                }
            }

            for(DiagramItem::TransformState state : endItem->linkWhere().keys()){
                tempRect = endItem->mapRectToScene(endItem->linkWhere()[state]);
                QRectF *magnetRect = new QRectF(tempRect.topLeft()-QPointF(20,20),tempRect.size()+QSize(40,40));
                if( magnetRect->contains(pathLine->line().p2()) ) {
                    endState = state;
                    qDebug()<<endState;
                    break;
                }
            }

            // qDebug()<<startState<<endState;
            if(startState && endState){
                DiagramPath *path = new DiagramPath(startItem,endItem,startState,endState);

                startItem->addPathes(path);
                startItem->marks[path] = "1" + QString::number(startState);
                qDebug()<<startItem->marks;
                endItem->addPathes(path);
                endItem->marks[path] = "0" + QString::number(endState);
                qDebug()<<endItem->marks;
                qDebug()<<startState<<endState;
                path->updatePath();
                path->setZValue(-1000.0);
                addItem(path);
                emit pathInserted(path);
            }
        }
        removeItem(pathLine);
        delete pathLine;
        setMode(premode);


        isInsertPath = false;
        setLinkVisible(false);
    }

    line = nullptr;
    pathLine = nullptr;
    QGraphicsScene::mouseReleaseEvent(mouseEvent);  // 保留原有的释放事件逻辑
}
//! [13]

//! [14]
bool DiagramScene::isItemChange(int type) const
{
    const QList<QGraphicsItem *> items = selectedItems();
    const auto cb = [type](const QGraphicsItem *item) { return item->type() == type; };
    return std::find_if(items.begin(), items.end(), cb) != items.end();
}
//! [14]
//! [15]
// 在 DiagramScene 类中添加
QGraphicsItem* DiagramScene::createItem(int type) {
    DiagramItem* newItem = new DiagramItem(static_cast<DiagramItem::DiagramType>(type),myItemMenu);
    addItem(newItem);
    return newItem;
}

QGraphicsTextItem* DiagramScene::createTextItem() {
    DiagramTextItem* newItem = new DiagramTextItem();
    addItem(newItem);
    return newItem;
}
void DiagramScene::setLinkVisible(bool b)   //设置全局所有DiagramItem显示连接点
{
    DiagramItem *item;
    QList<QGraphicsItem *> itemList = this->items();
    for(auto *it : itemList){
        if(it->type()== DiagramItem::Type){
            item = qgraphicsitem_cast<DiagramItem *>(it);
            item->showLink = b;
            item->update();
        }
    }
}
//! [15]

