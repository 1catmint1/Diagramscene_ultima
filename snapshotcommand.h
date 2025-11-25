#ifndef SNAPSHOTCOMMAND_H
#define SNAPSHOTCOMMAND_H

#include <QUndoCommand>
#include <QGraphicsView>
#include <QPixmap>
#include <QGraphicsPixmapItem>

class SnapshotCommand : public QUndoCommand {
public:
    SnapshotCommand(QGraphicsView *view, const QPixmap &pixmap, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    QGraphicsView *m_view;
    QPixmap m_pixmap;
    QGraphicsPixmapItem *m_pixmapItem;
};

#endif // SNAPSHOTCOMMAND_H
