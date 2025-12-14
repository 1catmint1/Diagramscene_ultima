// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DIAGRAMITEM_H
#define DIAGRAMITEM_H

#include <QGraphicsPixmapItem>
#include <QList>
#include<QBrush>
#include <QJsonObject>




QT_BEGIN_NAMESPACE
class QPixmap;
class QGraphicsSceneContextMenuEvent;
class QMenu;
class QPolygonF;
QT_END_NAMESPACE

class Arrow;
class DiagramPath;

//! [0]
class DiagramItem : public QGraphicsItem
{
public:
    bool isMoving = false;  // 用于标记是否正在拖动
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
    enum { Type = UserType + 15 };
    enum DiagramType { Step, Conditional, StartEnd, Io, circular,
                       Document,PredefinedProcess,StoredData,Memory,
                       SequentialAccessStorage,DirectAccessStorage,Disk,Card,
                       ManualInput,PerforatedTape,Display,Preparation,
                       ManualOperation,ParallelMode,Hexagon};
    DiagramType myDiagramType;
    DiagramItem(DiagramType diagramType, QMenu *contextMenu, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override; //重写boundingRect（）虚函数
    void setBrush(QColor &color);

    void removeArrow(Arrow *arrow);
    void removeArrows();

    DiagramType diagramType() const { return myDiagramType; }
    QPolygonF polygon() const { return myPolygon; }
    void addArrow(Arrow *arrow);

    QPixmap image() const;
    int type() const override { return Type; }

    void setBrush(QBrush *brush);
    void setFixedSize(const QSizeF &size);
    QColor m_color;
    QColor text_color;
    QString textContent ="请输入";
    QGraphicsTextItem *textItem;  // 添加文本框作为成员变量
    QList<DiagramPath *> pathes;
    QMap<DiagramPath*,QString> marks;

    void setRotationAngle(qreal angle);  // 设置旋转角度
    qreal rotationAngle() const;         // 获取当前旋转角度

    void setSize(QSizeF size);
    void setWidth(qreal width);
    void setHeight(qreal height);
    QSizeF getSize();
    void ableEvents();
    void disableEvents();
    void removePath(DiagramPath *path);
    void removePathes();
    QMap<TransformState, QRectF> rectWhere(); //绘制点
    QMap<TransformState,QRectF> linkWhere(); // 绘制连接点

    bool showLink=false;

    void addPathes(DiagramPath *path);
    void updatePathes();


protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override; //重载悬停函数

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override; //重载移动函数

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *) override; // 重载绘画函数


private:
    qreal m_rotationAngle;  // 用于存储当前图元的旋转角度
    QPolygonF myPolygon;
    QMenu *myContextMenu;
    QList<Arrow *> arrows;

    qreal m_border;
    QSizeF m_grapSize;   //boundingrect尺寸
    QSizeF m_minSize;    //最小尺寸

    bool isHover=true;
    bool isChange=true;






    TransformState m_tfState;
};
//! [0]

#endif // DIAGRAMITEM_H
