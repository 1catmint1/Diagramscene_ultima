#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QAction>
#include <QMenu>

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    MyGraphicsView(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    QAction *pAction;  // Declare the QAction pointer
};

#endif // MYGRAPHICSVIEW_H
