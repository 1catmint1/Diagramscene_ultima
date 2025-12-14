#include "mygraphicsview.h"
#include "mainwindow.h"

MyGraphicsView::MyGraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    pAction = new QAction(this);  // Initialize QAction
    connect(pAction, &QAction::triggered, this, &MainWindow::pasteItems);  // Connect the signal and slot
}

void MyGraphicsView::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    pAction = menu.addAction(tr("Paste"));

    QAction *selectedAction = menu.exec(event->screenPos());
    if (selectedAction == pAction) {
        // If you need to pass a parameter to pasteItems, you can do so here
        static_cast<MainWindow*>(parent())->pasteItems();
    }
}
