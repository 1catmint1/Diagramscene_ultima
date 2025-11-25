// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include "diagramitem.h"
#include "diagramtextitem.h"

#include <QGraphicsScene>
#include <QKeyEvent>



QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
class QMenu;
class QPointF;
class QGraphicsLineItem;
class QFont;
class QGraphicsTextItem;
class QColor;
QT_END_NAMESPACE

extern bool isInsertPath;

//! [0]
class DiagramScene : public QGraphicsScene
{
    Q_OBJECT

public:
    enum Mode { InsertItem, InsertLine, InsertText, MoveItem, InsertPath};

    explicit DiagramScene(QMenu *itemMenu, QObject *parent = nullptr);
    QFont font() const { return myFont; }
    QColor textColor() const { return myTextColor; }
    QColor itemColor() const { return myItemColor; }
    QColor lineColor() const { return myLineColor; }
    void setLineColor(const QColor &color);
    void setTextColor(const QColor &color);
    void setItemColor(const QColor &color);
    void setFont(const QFont &font);
    void setLinkVisible(bool b);

public slots:
    void setMode(Mode mode);
    void setItemType(DiagramItem::DiagramType type);
    void editorLostFocus(DiagramTextItem *item);
    QGraphicsItem* createItem(int type);
    QGraphicsTextItem* createTextItem();
signals:
    void itemInserted(DiagramItem *item);
    void textInserted(QGraphicsTextItem *item);
    void itemSelected(QGraphicsItem *item);
    void pathInserted(DiagramPath *path);

protected:
        // 重写键盘事件
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    bool isItemChange(int type) const;

    DiagramItem::DiagramType myItemType;
    QMenu *myItemMenu;
    QMenu *menu;
    Mode myMode;
    bool leftButtonDown;
    QPointF startPoint;
    QGraphicsLineItem *line;
    QFont myFont;
    DiagramTextItem *textItem;
    QColor myTextColor;
    QColor myItemColor;
    QColor myLineColor;
    QPointF beginpoint;//鼠标框选起始位置
    QPointF endpoint;//鼠标框选末位置
    bool ischeckingbox=false;//判断是否正在框选
    QGraphicsRectItem *rectItem = nullptr;

    QPointF alignPosition;//对齐
    bool isleft = false;     // 标记是否进行水平对齐
    bool istop = false;      // 标记是否进行垂直对齐
    bool isright = false;     // 标记是否进行水平对齐
    bool isbottom = false;      // 标记是否进行垂直对齐
    bool iscenterX = false;   // 标记是否进行中心对齐
    bool iscenterY = false;   // 标记是否进行中心对齐
    QGraphicsItem *movedItem = nullptr;  // 当前正在拖动的图元
    QGraphicsItem *alignedItem = nullptr;  // 当前对齐的图元
    Mode premode;
    QGraphicsLineItem *pathLine;
};
//! [0]

#endif // DIAGRAMSCENE_H
