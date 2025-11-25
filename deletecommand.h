#ifndef DELETECOMMAND_H
#define DELETECOMMAND_H


#include <QUndoCommand>
#include <QGraphicsItem>
#include <QGraphicsScene>

class DeleteCommand : public QUndoCommand {
public:
    DeleteCommand(QGraphicsItem *item, QGraphicsScene *scene, QUndoCommand *parent = nullptr);


    void undo() override;

    void redo() override;

private:
    QGraphicsItem *m_item;
    QGraphicsScene *m_scene;
    QPointF m_itemPos;
    QSizeF m_itemSize;
};

#endif // DELETECOMMAND_H


