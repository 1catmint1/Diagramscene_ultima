#include "snapshotcommand.h"

SnapshotCommand::SnapshotCommand(QGraphicsView *view, const QPixmap &pixmap, QUndoCommand *parent)
    : QUndoCommand(parent), m_view(view), m_pixmap(pixmap) {
    // 创建一个 QPixmapItem 但不添加到场景中，仅用于存储快照
    m_pixmapItem = new QGraphicsPixmapItem(m_pixmap);
}

void SnapshotCommand::undo() {
    m_view->scene()->clear(); // 清除当前场景
    qDebug()<<"清楚";
    m_view->scene()->addItem(m_pixmapItem); // 添加快照项到场景
}

void SnapshotCommand::redo() {
    m_view->scene()->clear(); // 清除当前场景
    m_view->scene()->addItem(m_pixmapItem); // 添加快照项到场景
}
