// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "diagramitem.h"

#include <QMainWindow>
#include <QPixmap>
#include "diagramitem.h"
#include<QStack>
#include "findreplacedialog.h"  // 包含新添加的查找和替换对话框
#include "diagramtextitem.h"// 确保包含了 DiagramTextItem 的头文件

class DiagramScene;

QT_BEGIN_NAMESPACE
class QAction;
class QToolBox;
class QSpinBox;
class QComboBox;
class QFontComboBox;
class QButtonGroup;
class QLineEdit;
class QGraphicsTextItem;
class QFont;
class QToolButton;
class QAbstractButton;
class QGraphicsView;
class QHBoxLayout;
QT_END_NAMESPACE

//! [0]
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();


public slots:
    void copyItems();
    void pasteItems(const QPointF &scenePos);
    void cutItems();
private slots:
    void backgroundButtonGroupClicked(QAbstractButton *button);
    void buttonGroupClicked(QAbstractButton *button);
    void deleteItem();
    void pointerGroupClicked();
    void bringToFront();
    void sendToBack();
    void itemInserted(DiagramItem *item);
    void textInserted(QGraphicsTextItem *item);
    void currentFontChanged(const QFont &font);
    void fontSizeChanged(const QString &size);
    void sceneScaleChanged(const QString &scale);
    void textButtonTriggered();
    void fillButtonTriggered();
    void lineButtonTriggered();
    void handleFontChange();
    void itemSelected(QGraphicsItem *item);
    void about();
    bool saveSceneAsImage();
    void closeEvent(QCloseEvent *event);
    void backgroundChanged(int index);
    void newScene();    //新加
    void sceneymChanged();//新加
    void closeScene(int index); //新加
    void showContextMenu(const QPoint &pos);
    void pasteItemsFromMenu();
    void sceneChanged();
    void openFindReplaceDialog(); // 打开查找和替换对话框
    void handleFindText(const QString &text);
    void handleReplaceText(const QString &findText, const QString &replaceText);
    void handleReplaceAllText(const QString &findText, const QString &replaceText);
    void combination();
    void cancelCombination();
    void savefile();
    void saveSaveFilePath(const QString &filePath);
    void changebackground();
    bool saveSceneAsImageOrSvg();
    QString loadSaveFilePath();
    void loadfile();
    QList<struct WriteDiagramItem *> getStructList();
    QList<struct WriteDiagramPath *> getStructList1();
    QString savefilestack();
    void loadfilestack(QString str);
    void autoCleanStack();
    void undo();
    void redo();


    // void undo();
    // void redo();


private:
    void saveSavePicPath(const QString &filePath);
    QString loadSavePicPath();

    void createToolBox();
    void createActions();
    void createMenus();
    void createToolbars();


    QWidget *createBackgroundCellWidget(const QString &text,
                                        const QString &image);
    QWidget *createCellWidget(const QString &text,
                              DiagramItem::DiagramType type);

    template<typename PointerToMemberFunction>
    QIcon createColorIcon(QColor color);

    DiagramScene *scene;
    QGraphicsView *view;
    QPointF scenePos;

    QAction *exitAction;
    QAction *addAction;
    QAction *deleteAction;
    QAction *saveSceneAction;
    QAction *newSceneAction;    //新加
    QAction *copyAction;
    QAction *pasteAction;
    QAction *cutAction;
    QAction *saveFileAction;
    QAction *loadFileAction;

    QAction *findAction;
    QAction *combineAction;
    QAction *cancelcombineAction;
    QAction *changeAction;
    QAction *toFrontAction;
    QAction *sendBackAction;
    QAction *aboutAction;
    QAction *undoAction;
    QAction *redoAction;


    QMenu *fileMenu;
    QMenu *itemMenu;
    QMenu *aboutMenu;

    QToolBar *textToolBar;
    QToolBar *findToolBar;
    QToolBar *colorToolBar;
    QToolBar *pointerToolbar;
    QToolBar *fileToolBar;

    QComboBox *sceneScaleCombo;
    QComboBox *backgroundComboBox;
    QComboBox *itemColorCombo;
    QComboBox *textColorCombo;
    QComboBox *fontSizeCombo;
    QFontComboBox *fontCombo;

    QToolBox *toolBox;
    QButtonGroup *buttonGroup;
    QButtonGroup *pointerTypeGroup;
    QButtonGroup *backgroundButtonGroup;
    QToolButton *fontColorToolButton;
    QToolButton *fillColorToolButton;
    QToolButton *lineColorToolButton;
    QAction *boldAction;
    QAction *underlineAction;
    QAction *italicAction;
    QAction *textAction;
    QAction *fillAction;
    QAction *lineAction;

    QHBoxLayout *layout;
    QTabWidget *tabwidget;
    QVector<DiagramScene*> sceneVector;
    QVector<QGraphicsView*> viewVector;

    QStack<QString> undoStack;
    QStack<QString> redoStack;

    FindReplaceDialog *findReplaceDialog;  // 查找和替换对话框指针
    DiagramTextItem *currentTextItem = nullptr;  // 当前查找的文本项
    int lastSearchPosition = -1;
    int path=0;
    int fileCount=0;
};
//! [0]

#endif // MAINWINDOW_H
