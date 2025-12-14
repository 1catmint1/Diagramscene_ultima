#include "deletecommand.h"
#include "diagramitem.h"

DeleteCommand::DeleteCommand(QGraphicsItem *item, QGraphicsScene *scene, QUndoCommand *parent)
    : QUndoCommand(parent), m_item(item), m_scene(scene) {
    m_itemPos = item->pos();
    m_itemSize = item->boundingRect().size();
}
void DeleteCommand::undo() {
    m_scene->addItem(m_item);
    m_item->setPos(m_itemPos);
    // m_item->setFixedSize(m_itemSize);

}

void DeleteCommand::redo() {
    m_scene->removeItem(m_item);
}
