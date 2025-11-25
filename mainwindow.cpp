//绘制流程图主窗口的地方

#include "arrow.h"
#include "diagramitem.h"
#include "diagramscene.h"
#include "diagramtextitem.h"
#include "mainwindow.h"
#include "deletecommand.h"
#include "diagramitemgroup.h"
#include "diagrampath.h"

#include <QtWidgets>

#include "arrow.h"
#include "diagramitem.h"
#include "diagramscene.h"
#include "diagramtextitem.h"
#include "mainwindow.h"
#include <QtWidgets>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QString>
#include <QList>
#include <QPlainTextEdit>
#include <QTextStream>

#include <QSvgGenerator>
#include <QGraphicsScene>
#include<diagrampath.h>


const int InsertTextButton = -1;
bool saveFileSuccess;

int globalTabCounter = 1; // 全局变量，用于跟踪标签页数量

//这个步骤没有用
QString savePicPath;                                        //全局变量 图片导出路径 用来实现图片便利导出
const QString savePathPicName = "lastPicLog.txt";           // 定义图片存储路径的文件名


// 保存 savePicPath 地址到文件
void  MainWindow::saveSavePicPath(const QString &filePath) {
    QFile file(savePathPicName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << filePath;
        file.close();
    }
}

QString MainWindow::loadSavePicPath() {
    QFile file(savePathPicName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString filePath = in.readAll();
        file.close();
        return filePath;
    }
    return QString(); // 如果文件不存在或读取失败，返回空字符串
}

//! [0]
MainWindow::MainWindow()
{
    createActions();
    createToolBox();    //左侧图形
    createMenus();      //上方菜单
    createToolbars();   //上方字体/颜色等

    findReplaceDialog = new FindReplaceDialog(this);//文本查找

    scene = new DiagramScene(itemMenu, this);
    scene->setSceneRect(QRectF(0, 0, 1920, 1080)); // 设置新场景的矩形区域
    scene->setBackgroundBrush(QPixmap(":/images/background4.png")); //默认纯白 我选的灰白网格


    //信号和槽 主要是组件被选择后的信号
    connect(scene, &DiagramScene::itemInserted,this, &MainWindow::itemInserted);
    connect(scene, &DiagramScene::textInserted,this, &MainWindow::textInserted);
    connect(scene, &DiagramScene::itemSelected,this, &MainWindow::itemSelected);

    connect(findReplaceDialog, &FindReplaceDialog::findText, this, &MainWindow::handleFindText);
    connect(findReplaceDialog, &FindReplaceDialog::replaceText, this, &MainWindow::handleReplaceText);
    connect(findReplaceDialog, &FindReplaceDialog::replaceAllText, this, &MainWindow::handleReplaceAllText);
    connect(scene, &DiagramScene::itemInserted,this, &MainWindow::savefilestack);
    connect(scene, &DiagramScene::textInserted,this, &MainWindow::savefilestack);
    connect(scene, &DiagramScene::pathInserted,this, &MainWindow::savefilestack);


    ///////////////////////////////////
    //这一段不建议进行注释处理 不认可能会导致内存报错 整个程序不能再构建
    layout = new QHBoxLayout;
    layout->addWidget(toolBox);
    view = new QGraphicsView(scene);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, &QGraphicsView::customContextMenuRequested, this, &MainWindow::showContextMenu);
    //这一段不建议进行注释处理 不认可能会导致内存报错 整个程序不能再构建
    ///////////////////////////////////


    //新加
    //采用 tab 的问题就是你每创造一个新的页面 对应的保存检测 和 redo undo 栈 都得额外来一份
    tabwidget = new QTabWidget();       //  一个组件 和 Sheet很相似 用来创造多个页面
    tabwidget->setTabsClosable(true);   //  组件可关闭

    view->centerOn(0, 0);
    QString tabTitle = QString("新页面%1").arg(globalTabCounter++);
    tabwidget->addTab(view, tabTitle);

    // connect(tabwidget,&QTabWidget::currentChanged,this, &MainWindow::sceneymChanged);     //新加，切换当前页面
    // connect(tabwidget,SIGNAL(tabCloseRequested(int)),this,SLOT(closeScene(int)));       //新加，关闭页面
    sceneVector.append(scene);
    viewVector.append(view);
    layout->addWidget(tabwidget);
    //新加完

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    setCentralWidget(widget);
    QIcon icon(":/images/logo.png");
    // QIcon icon(":/images/logo.ico");
    if (!icon.isNull()) {
        setWindowIcon(icon);
    } else {
        qDebug() << "Icon is null, check the resource path.";
    }

    setCentralWidget(widget);
    setWindowTitle(tr("流程图工程界面"));
    setUnifiedTitleAndToolBarOnMac(true);



    //////////////////////////////////////这一部分监听非常有必要 因为你不监听他检测不出来
    /// 相当于本质上就在执行这一个函数 你如果不开监听口就听不到了
    // 监听 QTabWidget 的 currentChanged 信号
    connect(tabwidget, &QTabWidget::currentChanged, this, &MainWindow::sceneChanged);
    // 监听 QTabWidget 的 tabCloseRequested 信号
    connect(tabwidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeScene);
}


// void MainWindow::undo() {
//     if (!undoStack->isEmpty()) {
//         QString stateFilePath = undoStack.pop();
//         loadState(stateFilePath);
//         redoStack.push(stateFilePath);
//     }
// }

// void MainWindow::redo() {
//     if (!redoStack.isEmpty()) {
//         QString stateFilePath = redoStack.pop();
//         loadState(stateFilePath);
//         undoStack.push(stateFilePath);
//     }
// }

QString saveFilePath;//全局变量 文件路径 用来实现文件便利读取
QString key = "123";

// DiagramItem 和 LineSegment 结构体的定义
struct  WriteDiagramItem {
    int x; // x坐标
    int y; // y坐标
    int width; // 宽度
    int height; // 高度
    int type; // 图形类型
    int rbg[4]; // 图形颜色(rbg三原色+透明度)
    QString internalText; // 图形内部Text的内容
    int itemtype;// 图形的类型
    QString texttype;// 图形内部文本的类型
    int textsize;//文字字号
    bool boldtype;//是否黑体
    bool itlatic;//是否斜体
    int textrbg[4];// 图形内部文字的color
};

struct ReadDiagramItem {
    int x; // x坐标
    int y; // y坐标
    int width; // 宽度
    int height; // 高度
    int type; // 图形类型
    int rbg[4]; // 图形颜色(rbg三原色+透明度)
    QString internalText; // 图形内部Text的内容
    int itemtype;// 图形的类型
    QString texttype;// 图形内部文本的类型
    int textsize;//文字字号
    bool boldtype;//是否黑体
    bool itlatic;//是否斜体
    int textrbg[4];// 图形内部文字的color
};

struct WriteDiagramPath
{
    int start;//开始item编号
    int startp;//开始item节点
    int end;//结束item编号
    int endp;//结束item节点
};

struct ReadDiagramPath
{
    int start;//开始item编号
    int startp;//开始item节点
    int end;//结束item编号
    int endp;//结束item节点
};








// /////////////////////////////////以下函数实现存储路径保存功能 防止因程序关闭而导致存储记忆消失
// 定义存储路径的文件名
const QString savePathFileName = "lastSavePathLog.txt";
// 保存 saveFilePath 到文件
void MainWindow::saveSaveFilePath(const QString &filePath) {
    QFile file(savePathFileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << filePath;
        file.close();
    }
}

QString MainWindow::loadSaveFilePath() {
    QFile file(savePathFileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString filePath = in.readAll();
        file.close();
        return filePath;
    }
    return QString(); // 如果文件不存在或读取失败，返回空字符串
}

//保存文件
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void MainWindow::savefile(){
    // 获取用户选择的文件路径
    saveFileSuccess = true; // 假设操作开始前是成功的
    QString textFile = QFileDialog::getSaveFileName(this, tr("保存工程文件"), "", tr("FC工程文件 (*.fcproj)"));
    // 如果用户取消了文件选择，则不执行任何操作
    if (textFile.isEmpty()) {
        saveFileSuccess = false;
        return;
    }
    // 存储保存文件的路径
    saveFilePath = textFile;
    // 保存 saveFilePath 到文件
    saveSaveFilePath(saveFilePath);
    // 创建 QFile 对象
    QFile file(textFile);
    // 以只写模式打开文件
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("保存失败"), tr("无法打开或写入工程信息."));
        saveFileSuccess = false;
        return;
    }
    // 创建 QTextStream 对象，并指定编码为 UTF-8
    QTextStream out(&file);
    // 获取 DiagramItem 结构体指针列表
    QList<WriteDiagramItem*> diagramItems = getStructList();  //这个是得到的图形的
    // 写入 DiagramItem 结构体列表的大小
    out << "DT_Size_" << diagramItems.size() << "\n";
    // 遍历 DiagramItem 结构体指针列表
    for (WriteDiagramItem* itemPtr : diagramItems) {
        // 替换 internalText 和 texttype 中的空格为星号
        QString modifiedInternalText = itemPtr->internalText.replace(" ", "*");
        QString modifiedTextType = itemPtr->texttype.replace(" ", "*");

        // 写入结构体成员
        out << itemPtr->x << " "
            << itemPtr->y << " "
            << itemPtr->width << " "
            << itemPtr->height << " "
            << itemPtr->type << " ";
        for (int i = 0; i < 4; ++i) {
            out << itemPtr->rbg[i] << " ";
        }
        out << modifiedInternalText << " " // 使用替换后的 internalText
            << itemPtr->itemtype << " "
            << modifiedTextType << " " // 使用替换后的 texttype
            << itemPtr->textsize << " "
            << itemPtr->boldtype << " "
            << itemPtr->itlatic << " ";
        for (int i = 0; i < 4; ++i) {
            out << itemPtr->textrbg[i] << " ";
        }
        out << "\n";
    }
    // 关闭文件
    file.close();
    // 获取 DiagramPath 结构体指针列表
    // 获取 DiagramPath 结构体指针列表
    QList<struct WriteDiagramPath *> diagramPathsPtrs = getStructList1();
    // 如果用户取消了文件选择或列表为空，则不执行任何操作
    if (textFile.isEmpty() || diagramPathsPtrs.isEmpty()) {
        saveFileSuccess = false;
        return;
    }
    // 以追加模式打开文件，以写入 DiagramPath 数据
    if (!file.open(QIODevice::Append)) {
        QMessageBox::critical(this, tr("保存失败"), tr("无法打开或写入工程信息."));
        saveFileSuccess = false;
        return;
    }
    // 写入 DiagramPath 结构体列表的大小
    out << "LN_Size_" << diagramPathsPtrs.size() << "\n";

    // 遍历 DiagramPath 结构体指针列表
    for (struct WriteDiagramPath *pathPtr : diagramPathsPtrs) {
        if (pathPtr) {
            // 写入结构体成员
            out << pathPtr->start << " "
                << pathPtr->startp << " "
                << pathPtr->end << " "
                << pathPtr->endp << "\n";
        }
    }
    // 关闭文件
    file.close();
}
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void MainWindow::loadfile() {
    // 从文件中读取 saveFilePath
    saveFilePath = loadSaveFilePath();
    // 设置打开文件对话框的起始目录为保存文件的路径
    QString textFile = QFileDialog::getOpenFileName(this, tr("打开文件"), saveFilePath, tr("FC工程文件 (*.fcproj);;All Files (*)"));

    // 如果用户取消了文件选择，则不执行任何操作
    if (textFile.isEmpty()) {
        return;
    }

    // 创建 QFile 对象
    QFile file(textFile);
    // 以只读模式打开文件
    if (file.open(QIODevice::ReadOnly)) {
        // 创建 QTextStream 对象
        QTextStream in(&file);

        // 创建新的列表用于存放读取的数据
        QList<ReadDiagramItem> readDiagramItems;

        // 读取 DiagramItem 结构体列表的大小
        QString sizeString;
        in >> sizeString;
        if (sizeString.startsWith("DT_Size_")) {
            quint32 diagramItemCount = sizeString.mid(8).toUInt(); // 去掉前缀并转换为整数
            for (quint32 i = 0; i < diagramItemCount; ++i) {
                ReadDiagramItem item;
                in >> item.x >> item.y >> item.width >> item.height >> item.type;
                for (int j = 0; j < 4; ++j) {
                    in >> item.rbg[j];
                }
                // 读取 internalText 并替换星号为空格
                QString internalText;
                in >> internalText;
                item.internalText = internalText.replace("*", " ");
                in >> item.itemtype;
                // 读取 texttype 并替换星号为空格
                QString textType;
                in >> textType;
                item.texttype = textType.replace("*", " ");
                in >> item.textsize;
                // 读取布尔值需要特殊处理
                QString boldTypeStr;
                in >> boldTypeStr;
                item.boldtype = (boldTypeStr == "true");

                QString itlaticStr;
                in >> itlaticStr;
                item.itlatic = (itlaticStr == "true");

                for (int j = 0; j < 4; ++j) {
                    in >> item.textrbg[j];
                }
                // 调试输出每个读取到的 DiagramItem
                qDebug() << "Read DiagramItem:";
                qDebug() << "x: " << item.x;
                qDebug() << "y: " << item.y;
                qDebug() << "width: " << item.width;
                qDebug() << "height: " << item.height;
                qDebug() << "type: " << item.type;
                qDebug() << "rbg: ";
                for (int i = 0; i < 4; ++i) {
                    qDebug() << item.rbg[i];
                }
                qDebug() << "internalText: " << item.internalText;
                qDebug() << "itemtype: " << item.itemtype;
                qDebug() << "texttype: " << item.texttype;
                qDebug() << "textsize: " << item.textsize;
                qDebug() << "boldtype: " << item.boldtype;
                qDebug() << "itlatic: " << item.itlatic;
                qDebug() << "textrbg: ";
                for (int i = 0; i < 4; ++i) {
                    qDebug() << item.textrbg[i];
                }
                readDiagramItems.append(item);
            }
        }

        // 创建新的列表用于存放读取的 DiagramPath 数据
        QList<ReadDiagramPath> readDiagramPaths;
        in >> sizeString;
        if (sizeString.startsWith("LN_Size_")) {
            quint32 diagramPathCount = sizeString.mid(8).toUInt(); // 去掉前缀并转换为整数
            for (quint32 i = 0; i < diagramPathCount; ++i) {
                ReadDiagramPath path;
                in >> path.start >> path.startp >> path.end >> path.endp;
                // 调试输出每个读取到的 DiagramPath
                qDebug() << "Read DiagramPath:";
                qDebug() << "start: " << path.start;
                qDebug() << "startp: " << path.startp;
                qDebug() << "end: " << path.end;
                qDebug() << "endp: " << path.endp;
                readDiagramPaths.append(path);
            }
        }
        newScene();
        QList<DiagramItem*> DiagramItemList;
        foreach (ReadDiagramItem item,readDiagramItems) {
            qDebug()<<"画";
            DiagramItem *item1=new DiagramItem(static_cast<DiagramItem::DiagramType>(item.itemtype),itemMenu);
            item1->setPos(QPoint(item.x,item.y));
            item1->setFixedSize(QSize(item.width,item.height));
            QColor itemcolor(item.rbg[0],item.rbg[2],item.rbg[1],item.rbg[3]);
            QColor textcolor(item.textrbg[0],item.textrbg[2],item.textrbg[1],item.textrbg[3]);
            item1->m_color=itemcolor;
            item1->textItem->setPlainText(item.internalText);
            item1->textItem->font().setPointSize(item.textsize);
            item1->textItem->font().setFamily(item.texttype);
            item1->textItem->font().setBold(item.boldtype);
            item1->textItem->font().setItalic(item.itlatic);
            item1->textItem->setDefaultTextColor(textcolor);
            scene->addItem(item1);
            DiagramItemList.append(item1);
        }

        int size = DiagramItemList.size();
        qDebug()<<"size: "<< size;
        foreach (ReadDiagramPath item,readDiagramPaths) {
            DiagramItem *startItem = DiagramItemList.at(item.start-1 );
            DiagramItem *endItem = DiagramItemList.at(item.end-1 );
            qDebug()<<scene->items();
            DiagramItem::TransformState startState = static_cast<DiagramItem::TransformState>(item.startp);
            DiagramItem::TransformState endState = static_cast<DiagramItem::TransformState>(item.endp);

            DiagramPath *item1=new DiagramPath(startItem,endItem,startState,endState);

            startItem->addPathes(item1);
            startItem->marks[item1] = "1" + QString::number(startState);
            qDebug()<<startItem->marks;
            endItem->addPathes(item1);
            endItem->marks[item1] = "0" + QString::number(endState);
            qDebug()<<endItem->marks;
            qDebug()<<startState<<endState;
            item1->updatePath();
            item1->setZValue(-1000.0);

            scene->addItem(item1);

        }
        // 提示用户读取成功
        QMessageBox::information(this, tr("加载完成"), tr("成功加载工程."));
    } else {
        // 文件打开失败，提示用户错误信息
        QMessageBox::critical(this, tr("加载失败"), tr("无法打开或读取文件信息."));
    }
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
QList<struct WriteDiagramItem *> MainWindow::getStructList() {
    QList<struct WriteDiagramItem *> DiagramItemList;
    int p=0;
    foreach(QGraphicsItem *item, scene->items()) {
        DiagramItem *item1 = qgraphicsitem_cast<DiagramItem *>(item);
        if (!item1) {
            // 如果没有成功转换，可能是类型不匹配，可以在这里添加一个警告
            qWarning() << "Failed to cast item to DiagramItem";
            continue;
        }
        struct WriteDiagramItem *item0 = new WriteDiagramItem();
        p++;
        item0->x = item1->pos().x();
        item0->y = item1->pos().y();
        item0->width = item1->boundingRect().width()*0.8;
        item0->height = item1->boundingRect().height()*0.72;
        item0->rbg[0] = item1->m_color.red();
        item0->rbg[1] = item1->m_color.blue();
        item0->rbg[2] = item1->m_color.green();
        item0->rbg[3] = item1->m_color.alpha();
        item0->internalText = item1->textItem->toPlainText();
        item0->itemtype = item1->myDiagramType;
        item0->texttype = item1->textItem->font().family();
        item0->textsize = item1->textItem->font().pointSize();
        item0->boldtype = item1->textItem->font().bold();
        item0->itlatic = item1->textItem->font().italic();
        item0->textrbg[0] = item1->textItem->defaultTextColor().red();
        item0->textrbg[1] = item1->textItem->defaultTextColor().blue();
        item0->textrbg[2] = item1->textItem->defaultTextColor().green();
        item0->textrbg[3] = item1->textItem->defaultTextColor().alpha();
        qDebug()<<"type:"<<item1->myDiagramType;
        qDebug()<<"item_marks: "<<item1->marks;

        foreach (DiagramPath* line, item1->pathes) {
            if(item1->marks[line][0] == '1' ){
                line->start = p;
                line->startp = item1->marks[line][1].digitValue();
                qDebug()<<line;
                qDebug()<<"start:"<<line->start;
                qDebug()<<"startp"<<line->startp;
            }
            else if(item1->marks[line][0] == '0'){
                line->end = p;
                line->endp = item1->marks[line][1].digitValue();
                qDebug()<<line;
                qDebug()<<"end:"<<line->end;
                qDebug()<<"endp"<<line->endp;
            }
        }

        DiagramItemList.append(item0);
    }
    return DiagramItemList;
}
/////////////////////////////////////
QList<struct WriteDiagramPath *> MainWindow::getStructList1() {
    QList<struct WriteDiagramPath *> DiagramItemList;
    foreach(QGraphicsItem *item, scene->items()) {
        DiagramPath *item1 = qgraphicsitem_cast<DiagramPath *>(item);
        if (!item1) {
            // 如果没有成功转换，可能是类型不匹配，可以在这里添加一个警告
            continue;
        }
        struct WriteDiagramPath *item0 = new WriteDiagramPath();
        item0->start=item1->start;
        item0->startp=item1->startp;
        item0->end=item1->end;
        item0->endp=item1->endp;
        qDebug()<<"load path: ";
        qDebug()<<"start: "<<item0->start;
        qDebug()<<"startp:"<<item0->startp;
        qDebug()<<"end: "<<item0->end;
        qDebug()<<"endp: "<<item0->endp;
        DiagramItemList.append(item0);
    }
    return DiagramItemList;
}
//组合
void MainWindow::combination(){
    DiagramItemGroup *group = new DiagramItemGroup();
    foreach (QGraphicsItem *item, scene->selectedItems()) {
        DiagramItem *item1 = qgraphicsitem_cast<DiagramItem *>(item);
        if(item1){
            item1->disableEvents();
            group->addItem(item);
            scene->removeItem(item);
        }
    }
    scene->addItem(group);
    scene->update();
    group->setPos(group->getTopLeft());
}

void MainWindow::cancelCombination(){
    if(scene->selectedItems().isEmpty()) return;
    DiagramItemGroup *group = qgraphicsitem_cast<DiagramItemGroup *>(scene->selectedItems().first());
    if(group){
        scene->removeItem(scene->selectedItems().first());
        for(int t=0;group->childItems().size()>0;t++) {
            DiagramItem *item1 = qgraphicsitem_cast<DiagramItem *>(group->childItems().at(0));
            if(item1){
                scene->addItem(item1);
                item1->setPos(group->itemScenePos.at(t));
                item1->ableEvents();
            }
        }
        scene->update();
    }
}
//查找文件
void MainWindow::openFindReplaceDialog()
{
    if (findReplaceDialog) {
        findReplaceDialog->show();  // 显示查找和替换对话框
    }
}
void MainWindow::handleFindText(const QString &text)
{
    bool found = false;  // 用于指示是否找到文本
    QList<QGraphicsItem *> items = scene->items();  // 获取场景中的所有文本项
    int startIndex = 0;  // 从哪个文本项开始查找

    // 如果上次已经查找到某个文本框了，继续从该文本框及其后面的文本框开始查找
    if (currentTextItem) {
        // 取消当前高亮
        QTextCursor cursor = currentTextItem->textCursor();
        cursor.clearSelection();  // 取消当前选中的文本
        currentTextItem->setTextCursor(cursor);  // 应用更新后的光标

        startIndex = items.indexOf(currentTextItem);  // 从上次查找到的文本框继续
    }

    // 从当前文本框的下一个字符开始继续查找
    for (int i = startIndex; i < items.size(); ++i) {
        if (DiagramTextItem *textItem = qgraphicsitem_cast<DiagramTextItem *>(items[i])) {
            QString content = textItem->toPlainText();  // 获取文本内容

            // 如果是从当前文本项继续查找，使用 lastSearchPosition 来避免从头查找
            int searchStartPosition = (textItem == currentTextItem) ? lastSearchPosition + 1 : 0;
            int index = content.indexOf(text, searchStartPosition);

            if (index != -1) {  // 找到匹配的文本
                // 使用 QTextCursor 选中文本
                QTextCursor cursor = textItem->textCursor();
                cursor.setPosition(index);              // 设置光标位置到找到的文本开始处
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, text.length());  // 选中查找到的文本
                textItem->setTextCursor(cursor);        // 应用新的文本光标

                textItem->setSelected(true);            // 选中整个文本框（可选）
                view->ensureVisible(textItem);          // 将查找到的文本项滚动到视图中可见的位置

                // 保存当前查找到的文本框和位置
                currentTextItem = textItem;
                lastSearchPosition = index + text.length();  // 更新上次查找结束的位置

                found = true;
                break;  // 找到后退出
            }
        }
    }

    // 如果没有找到任何匹配项，重置查找状态，并提示用户
    if (!found) {
        currentTextItem = nullptr;
        lastSearchPosition = -1;
        QMessageBox::information(this, tr("查找结束"), tr("未找到更多的匹配项。"));
    }
}

void MainWindow::handleReplaceText(const QString &findText, const QString &replaceText)
{
    // 遍历场景中的所有文本项，替换指定文本
    foreach (QGraphicsItem *item, scene->items()) {
        if (DiagramTextItem *textItem = qgraphicsitem_cast<DiagramTextItem *>(item)) {
            QString currentText = textItem->toPlainText();
            if (currentText.contains(findText)) {
                textItem->setPlainText(currentText.replace(findText, replaceText));  // 替换找到的文本
                break;  // 替换第一个匹配项后退出循环
            }
        }
    }
}

void MainWindow::handleReplaceAllText(const QString &findText, const QString &replaceText)
{
    // 遍历场景中的所有文本项，替换所有匹配的文本
    foreach (QGraphicsItem *item, scene->items()) {
        if (DiagramTextItem *textItem = qgraphicsitem_cast<DiagramTextItem *>(item)) {
            QString currentText = textItem->toPlainText();
            if (currentText.contains(findText)) {
                textItem->setPlainText(currentText.replace(findText, replaceText));  // 替换所有匹配项
            }
        }
    }
}

//! [0]
void MainWindow::sceneChanged() // 新加
{
    qDebug() << "切换到了场景" << tabwidget->currentIndex();
    int currentIndex = tabwidget->currentIndex();
    if (currentIndex >= 0 && currentIndex < sceneVector.size()) {
        scene = sceneVector[currentIndex];
        view = viewVector[currentIndex];
        // 重新连接信号和槽
        connect(scene, &DiagramScene::itemInserted, this, &MainWindow::itemInserted);
        connect(scene, &DiagramScene::textInserted, this, &MainWindow::textInserted);
        connect(scene, &DiagramScene::itemSelected, this, &MainWindow::itemSelected);
    }
}
//画布菜单
void MainWindow::showContextMenu(const QPoint &pos) {
    // 检查是否有选中的图形项
    bool hasSelectedItems = !scene->selectedItems().isEmpty();
    // 创建菜单
    QMenu menu(this);
    bool hasdiagramitem=false;
    if (!hasSelectedItems) {
        // 用户在空白处右击，添加特定的菜单项
        menu.addAction(pasteAction);
        menu.addAction(undoAction);
        menu.addAction(redoAction);

        // menu.addAction(undoAction);
        // menu.addAction(redoAction);
    } else {
        foreach (QGraphicsItem *item, scene->selectedItems()) {
            DiagramItem *item1=qgraphicsitem_cast<DiagramItem *>(item);
            if(item1){
                hasdiagramitem=true;
                break;
            }
        }
        if(hasdiagramitem){
            foreach (QAction *action, itemMenu->actions()) {
                menu.addAction(action);
            }
        }
    }
    // 显示菜单
    menu.exec(view->mapToGlobal(pos));
}
//复制
void MainWindow::copyItems() {
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    QMap<DiagramItem*,int> idmap;
    int p = 0;
    foreach (QGraphicsItem *item, scene->selectedItems()) {
        if (DiagramItem *diagramItem = dynamic_cast<DiagramItem*>(item)) {
            QPointF itemPos = diagramItem->pos(); // 获取图形项的位置
            QColor itemColor = diagramItem->m_color; // 获取图形项的颜色
            QSizeF itemSize = diagramItem->boundingRect().size(); // 获取图形项的大小
            QString content =  diagramItem->textItem->toPlainText();
            QColor textColor = diagramItem->textItem->defaultTextColor();
            QFont font =diagramItem->textItem->font();
            idmap[diagramItem] = p; //item的编号
            dataStream << static_cast<int>(diagramItem->myDiagramType)
                       << itemPos.x() << itemPos.y()
                       << itemColor.rgba() // 颜色以RGBA形式存储
                       << itemSize.width() << itemSize.height() << content <<textColor
                       <<font<<p++; // 存储大小
            // QMimeData *mimeData = new QMimeData();
            // mimeData->setData("application/x-diagramscene-item-type", itemData);
            // QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
            qDebug() << "复制成功";
        }
    }

    QByteArray pathData;
    QDataStream pathStream(&pathData,QIODevice::WriteOnly);

    foreach (QGraphicsItem *item, scene->selectedItems()) {
        if(DiagramPath *path = dynamic_cast<DiagramPath*>(item)){
            if(scene->selectedItems().contains(path->getStartItem()) &&
                scene->selectedItems().contains(path->getEndItem())){
                int start = idmap[path->getStartItem()];
                int end = idmap[path->getEndItem()];
                QChar startc = path->getStartItem()->marks[path][1];
                QChar endc = path->getEndItem()->marks[path][1];
                int startp = startc.digitValue();
                int endp = endc.digitValue();
                pathStream<<start<<end<<startp<<endp;
                // QMimeData *mimeData = new QMimeData();
                // mimeData->setData("application/x-digramscene-path-type",itemData);
                // QApplication::clipboard()->setMimeData(mimeData,QClipboard::Clipboard);
                qDebug()<<"线复制成功";
            }
        }
    }

    QMimeData *mimeData = new QMimeData();
    mimeData->setData("application/x-diagramscene-item-type",itemData);
    mimeData->setData("application/x-digramscene-path-type",pathData);
    QApplication::clipboard()->setMimeData(mimeData,QClipboard::Clipboard);

}

void MainWindow::cutItems() {
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);

    QList<QGraphicsItem*> itemsToCut; // 用于存储要剪切的图形项

    foreach (QGraphicsItem *item, scene->selectedItems()) {
        if (DiagramItem *diagramItem = dynamic_cast<DiagramItem*>(item)) {
            QPointF itemPos = diagramItem->pos(); // 获取图形项的位置
            QColor itemColor = diagramItem->m_color; // 获取图形项的颜色
            QSizeF itemSize = diagramItem->boundingRect().size(); // 获取图形项的大小
            QString content =  diagramItem->textItem->toPlainText();
            QColor textColor = diagramItem->textItem->defaultTextColor();
            QFont font =diagramItem->textItem->font();

            dataStream << static_cast<int>(diagramItem->myDiagramType)
                       << itemPos.x() << itemPos.y()
                       << itemColor.rgba() // 颜色以RGBA形式存储
                       << itemSize.width() << itemSize.height() << content <<textColor<<font; // 存储大小

            itemsToCut.append(item); // 将图形项添加到剪切列表
            QMimeData *mimeData = new QMimeData();
            mimeData->setData("application/x-diagramscene-item-type", itemData);
            QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
            qDebug() << "剪切成功";
        }
    }
    foreach (QGraphicsItem *item, itemsToCut) {
        scene->removeItem(item);
        delete item;
    }

}

void MainWindow::pasteItems(const QPointF &scenePos) {
    QMap<int,DiagramItem*> idmap;
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    if (mimeData && mimeData->hasFormat("application/x-diagramscene-item-type")) {
        qDebug()<<"enter item paste";
        QByteArray itemData = mimeData->data("application/x-diagramscene-item-type");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QPointF firstItemOffset;
        bool isFirstItem = true;
        while (!dataStream.atEnd()) {
            int typeInt;
            qreal x, y;
            quint32 color;
            qreal width, height;
            QString str;
            QColor textcolor;
            QFont font;
            int id;
            dataStream >> typeInt >> x >> y >> color >> width >> height>>str>>textcolor>>font>>id;

            QGraphicsItem *newItem = scene->createItem(typeInt); // 直接使用读取的位置

            if (DiagramItem *newDiagramItem = dynamic_cast<DiagramItem*>(newItem)) {
                QColor itemColor = QColor(color); // 从RGBA值创建QColor对象
                QSizeF itemSize(width*0.8, height*0.72); // 创建大小

                newDiagramItem->setBrush(itemColor); // 设置颜色
                newDiagramItem->setFixedSize(itemSize); // 设置固定大小
                newDiagramItem->textItem->setPlainText(str);//设置文字
                newDiagramItem->textItem->setDefaultTextColor(textcolor);
                newDiagramItem->textItem->setFont(font);

                idmap[id] = newDiagramItem;

                if (isFirstItem) {
                    firstItemOffset = QPointF(x, y);
                    newDiagramItem->setPos(scenePos);
                    isFirstItem = false;
                } else {
                    QPointF offset = QPointF(x, y) - firstItemOffset;
                    newDiagramItem->setPos(scenePos + offset);
                }

                scene->addItem(newItem);
            }
        }
        qDebug()<<"add item finished";
    }

    if(mimeData->hasFormat("application/x-digramscene-path-type")){
        qDebug()<<"enter path paste";
        QByteArray pathData = mimeData->data("application/x-digramscene-path-type");
        QDataStream dataStream(&pathData,QIODevice::ReadOnly);
        while(!dataStream.atEnd()){
            int start;
            int end;
            int startp;
            int endp;
            dataStream>>start>>end>>startp>>endp;
            DiagramItem *startItem = idmap[start];
            DiagramItem *endItem = idmap[end];
            DiagramItem::TransformState startState = static_cast<DiagramItem::TransformState>(startp);
            DiagramItem::TransformState endState = static_cast<DiagramItem::TransformState>(endp);

            DiagramPath *path = new DiagramPath(startItem,endItem,startState,endState);
            startItem->addPathes(path);
            startItem->marks[path] = "1" + QString::number(startp);
            endItem->addPathes(path);
            endItem->marks[path] = "0" + QString::number(endp);
            path->updatePath();
            path->setZValue(-1000.0);
            scene->addItem(path);
        }
    }else{
        qDebug()<<"have no path-type";
    }
}


void MainWindow::pasteItemsFromMenu() {
    // QPointF scenePos = view->mapToScene(QCursor::pos() - view->pos()); // 获取鼠标当前位置的场景坐标
    QPoint globalMousePos = QCursor::pos();
    // 转换为视图坐标
    QPoint viewMousePos = view->mapFromGlobal(globalMousePos);
    // 转换为场景坐标
    QPointF scenePos = view->mapToScene(viewMousePos);
    pasteItems(scenePos);
}

//保存为图片
bool MainWindow::saveSceneAsImage() {
    savePicPath = loadSavePicPath();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"),savePicPath, tr("PNG Files (*.png);;JPEG Files (*.jpg *.jpeg);;All Files (*)"));

    if (!fileName.isEmpty()) {
        // 存储保存文件的路径
        savePicPath = fileName;
        // 保存 saveFilePath 到文件
        saveSavePicPath(savePicPath);
        QPixmap pixmap = view->grab(); // 使用视图的grab方法
        if (!pixmap.save(fileName)) {
            QMessageBox::warning(this, tr("Error"), tr("Unable to save the image."));
        } else {
            QMessageBox::information(this, tr("Success"), tr("Image saved successfully."));
        }
    }
    return true;
}

//关闭提示
void MainWindow::closeEvent(QCloseEvent *event) {
    QMessageBox messageBox(QMessageBox::Warning, tr("工程文件未保存"), tr("是否保存工程文件？"),
                           QMessageBox::NoButton, this);
    QPushButton *saveButton = messageBox.addButton(tr("保存"), QMessageBox::ActionRole);
    QPushButton *discardButton = messageBox.addButton(tr("不保存"), QMessageBox::ActionRole);
    QPushButton *cancelButton = messageBox.addButton(QMessageBox::Cancel);

    messageBox.exec();

    if (messageBox.clickedButton() == saveButton) {
        savefile(); // 调用 savefile() 函数，它将改变 saveFileSuccess 的值
        if (saveFileSuccess) { // 检查 saveFileSuccess 的值
            event->accept();
        } else {
            event->ignore();
        }
    } else if (messageBox.clickedButton() == discardButton) {
        event->accept();
    } else if (messageBox.clickedButton() == cancelButton){
        event->ignore();
    }
}


//! [1]
//画布按钮点击其一，更新画布样式
void MainWindow::backgroundButtonGroupClicked(QAbstractButton *button)
{
    const QList<QAbstractButton *> buttons = backgroundButtonGroup->buttons();
    for (QAbstractButton *myButton : buttons) {
        if (myButton != button)
            button->setChecked(false);
    }
    QString text = button->text();
    if (text == tr("Blue Grid"))
        scene->setBackgroundBrush(QPixmap(":/images/background1.png"));
    else if (text == tr("White Grid"))
        scene->setBackgroundBrush(QPixmap(":/images/background2.png"));
    else if (text == tr("Gray Grid"))
        scene->setBackgroundBrush(QPixmap(":/images/background3.png"));
    else
        scene->setBackgroundBrush(QPixmap(":/images/background4.png"));

    scene->update();
    view->update();
}
//! [1]

//! [2]
void MainWindow::buttonGroupClicked(QAbstractButton *button)
{
    const QList<QAbstractButton *> buttons = buttonGroup->buttons();
    for (QAbstractButton *myButton : buttons) {
        if (myButton != button)
            button->setChecked(false);
    }
    const int id = buttonGroup->id(button);
    if (id == InsertTextButton) {
        scene->setMode(DiagramScene::InsertText);
    } else {
        scene->setItemType(DiagramItem::DiagramType(id));
        scene->setMode(DiagramScene::InsertItem);
    }
}
//! [2]

//! [3]
void MainWindow::deleteItem()
{
    qDebug()<<"enter delete";
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    for (QGraphicsItem *item : std::as_const(selectedItems)) {
        if (item->type() == Arrow::Type) {
            scene->removeItem(item);
            Arrow *arrow = qgraphicsitem_cast<Arrow *>(item);
            arrow->startItem()->removeArrow(arrow);
            arrow->endItem()->removeArrow(arrow);
            delete item;
        }
    }
    selectedItems = scene->selectedItems();
    for (QGraphicsItem *item : std::as_const(selectedItems)) {
        if (item->type() == DiagramItem::Type){
            DiagramItem *item1 = qgraphicsitem_cast<DiagramItem *>(item);
            item1->removeArrows();
            item1->removePathes();
            // savefilestack();
            scene->removeItem(item1);
            // savefilestack();
            delete item1;
        }
    }
    selectedItems = scene->selectedItems();
    for (QGraphicsItem *item : std::as_const(selectedItems)) {
        if (item->type() == DiagramPath::Type) {
            // qDebug()<<"asd1";
            savefilestack();
            scene->removeItem(item);
            savefilestack();
            DiagramPath *path = qgraphicsitem_cast<DiagramPath *>(item);
            path->getStartItem()->removePath(path);
            path->getEndItem()->removePath(path);
            delete item;
        }
    }
}
//! [3]

//! [4]
void MainWindow::pointerGroupClicked()
{
    scene->setMode(DiagramScene::Mode(pointerTypeGroup->checkedId()));
}
//! [4]

//! [5]
void MainWindow::bringToFront()
{
    if (scene->selectedItems().isEmpty())
        return;

    QGraphicsItem *selectedItem = scene->selectedItems().first();
    const QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

    qreal zValue = 0;
    for (const QGraphicsItem *item : overlapItems) {
        if (item->zValue() >= zValue && item->type() == DiagramItem::Type)
            zValue = item->zValue() + 0.1;
    }
    selectedItem->setZValue(zValue);
}
//! [5]

//! [6]
void MainWindow::sendToBack()
{
    if (scene->selectedItems().isEmpty())
        return;

    QGraphicsItem *selectedItem = scene->selectedItems().first();
    const QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

    qreal zValue = 0;
    for (const QGraphicsItem *item : overlapItems) {
        if (item->zValue() <= zValue && item->type() == DiagramItem::Type)
            zValue = item->zValue() - 0.1;
    }
    selectedItem->setZValue(zValue);
}
//! [6]

//! [7]
void MainWindow::itemInserted(DiagramItem *item)
{
    pointerTypeGroup->button(int(DiagramScene::MoveItem))->setChecked(true);
    scene->setMode(DiagramScene::Mode(pointerTypeGroup->checkedId()));
    buttonGroup->button(int(item->diagramType()))->setChecked(false);
}
//! [7]

//! [8]
void MainWindow::textInserted(QGraphicsTextItem *)
{
    buttonGroup->button(InsertTextButton)->setChecked(false);
    scene->setMode(DiagramScene::Mode(pointerTypeGroup->checkedId()));
}
//! [8]

//! [9]
void MainWindow::currentFontChanged(const QFont &)
{
    handleFontChange();
}
//! [9]

//! [10]
void MainWindow::fontSizeChanged(const QString &)
{
    handleFontChange();
}
//! [10]

//! [11]
void MainWindow::sceneScaleChanged(const QString &scale)
{
    double newScale = scale.left(scale.indexOf(tr("%"))).toDouble() / 100.0;
    QTransform oldMatrix = view->transform();
    view->resetTransform();
    view->translate(oldMatrix.dx(), oldMatrix.dy());
    view->scale(newScale, newScale);
}
//! [11]

void MainWindow::textButtonTriggered()
{
    QColor color = QColorDialog::getColor(Qt::white, this);
    if(color.isValid()){
        scene->setTextColor(qvariant_cast<QColor>(color));
    }
}
//! [15]

//! [16]
void MainWindow::fillButtonTriggered()
{
    QColor color = QColorDialog::getColor(Qt::white, this);
    if(color.isValid()){
        scene->setItemColor(qvariant_cast<QColor>(color));
    }
}
//! [16]

//! [17]
void MainWindow::lineButtonTriggered()
{
    QColor color = QColorDialog::getColor(Qt::white, this);
    if(color.isValid()){
        scene->setLineColor(qvariant_cast<QColor>(color));
    }
}
//! [18]
void MainWindow::handleFontChange()
{
    QFont font = fontCombo->currentFont();
    font.setPointSize(fontSizeCombo->currentText().toInt());
    font.setWeight(boldAction->isChecked() ? QFont::Bold : QFont::Normal);
    font.setItalic(italicAction->isChecked());
    font.setUnderline(underlineAction->isChecked());

    scene->setFont(font);
}
//! [18]

//! [19]
void MainWindow::itemSelected(QGraphicsItem *item)
{
    DiagramTextItem *textItem =
        qgraphicsitem_cast<DiagramTextItem *>(item);

    QFont font = textItem->font();
    fontCombo->setCurrentFont(font);
    fontSizeCombo->setEditText(QString().setNum(font.pointSize()));
    boldAction->setChecked(font.weight() == QFont::Bold);
    italicAction->setChecked(font.italic());
    underlineAction->setChecked(font.underline());
}
//! [19]

//帮助界面
void MainWindow::about()
{
    QMessageBox::about(this, tr("关于本流程图编辑器"),
                       tr("本 <b>FreeChats</b> 编辑器由"
                          "董壮志小组成员共同开发"));
}

void MainWindow::newScene() // 新加
{

    // 创建新的场景和视图
    DiagramScene *newScene = new DiagramScene(itemMenu, this);
    newScene->setSceneRect(QRectF(0, 0, 1920, 1080)); // 设置新场景的矩形区域
    newScene->setBackgroundBrush(QPixmap(":/images/background4.png")); // 设置背景

    // 创建新的视图并关联到新场景
    QGraphicsView *newView = new QGraphicsView(newScene);
    newView->setRenderHint(QPainter::Antialiasing); // 设置抗锯齿，提高渲染质量

    // 设置视图中心，使其与场景的左上角对齐
    newView->centerOn(0, 0);

    // 设置新标签页的标题
    QString tabTitle = QString("新页面%1").arg(globalTabCounter++);
    int index=tabwidget->addTab(newView, tabTitle); // 将新视图添加到标签页中

    // 存储新场景和视图以便管理
    sceneVector.append(newScene);
    viewVector.append(newView);
    tabwidget->setCurrentIndex(index);
    // 连接信号和槽，确保场景改变时能够更新
    connect(newScene, &DiagramScene::itemInserted, this, &MainWindow::itemInserted);
    connect(newScene, &DiagramScene::textInserted, this, &MainWindow::textInserted);
    connect(newScene, &DiagramScene::itemSelected, this, &MainWindow::itemSelected);

    // 通知主窗口场景已改变
    sceneChanged();
}


void MainWindow::sceneymChanged() //新加
{
    // qDebug() << "change to tab" << tabwidget->currentIndex();
    // scene = sceneVector[tabwidget->currentIndex()];
    // view = viewVector[tabwidget->currentIndex()];
    // connect(scene, &DiagramScene::itemInserted,this, &MainWindow::itemInserted);
    // connect(scene, &DiagramScene::textInserted,this, &MainWindow::textInserted);
    // connect(scene, &DiagramScene::itemSelected,this, &MainWindow::itemSelected);
}


void MainWindow::closeScene(int index)  // 新加
{
    if(tabwidget->count() <= 1) {
        qDebug() << "不能关闭最后一个标签页";
        return; // 如果是，则不允许关闭
    }
    qDebug() << "要求关闭场景" << index;
    // 断开信号和槽连接
    DiagramScene *sceneToRemove = sceneVector.at(index);
    disconnect(sceneToRemove, &DiagramScene::itemInserted, this, &MainWindow::itemInserted);
    disconnect(sceneToRemove, &DiagramScene::textInserted, this, &MainWindow::textInserted);
    disconnect(sceneToRemove, &DiagramScene::itemSelected, this, &MainWindow::itemSelected);
    // 从向量中移除场景和视图
    sceneVector.removeAt(index);
    viewVector.removeAt(index);
    // 从标签页中移除
    tabwidget->removeTab(index);
    // 更新当前场景和视图
    sceneChanged();
}

//! [20]
//左侧的工具栏
//! [21]
void MainWindow::createToolBox()
{
    buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(false);
    connect(buttonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::buttonGroupClicked);
    QGridLayout *layout = new QGridLayout;
    //这个应该是用来控制都有啥玩意的
    layout->addWidget(createCellWidget(tr("判断"), DiagramItem::Conditional), 0, 1);                                  //判断
    layout->addWidget(createCellWidget(tr("处理"), DiagramItem::Step),1, 0);                                         //处理
    layout->addWidget(createCellWidget(tr("数据"), DiagramItem::Io), 1, 1);                                         //数据
    layout->addWidget(createCellWidget(tr("连接符"),DiagramItem::circular),2,0);                                     //连接符
    layout->addWidget(createCellWidget(tr("端点符"),DiagramItem::StartEnd),2,1);                                     //端点符
    layout->addWidget(createCellWidget(tr("文件"),DiagramItem::Document),3,0);                                       //文件
    layout->addWidget(createCellWidget(tr("既定处理"),DiagramItem::PredefinedProcess),3,1);                          //既定处理
    layout->addWidget(createCellWidget(tr("存储数据"),DiagramItem::StoredData),4,0);                                  //存储数据
    layout->addWidget(createCellWidget(tr("内存储器"),DiagramItem::Memory),4,1);                                       //内存储器
    layout->addWidget(createCellWidget(tr("顺序存取存储器"),DiagramItem::SequentialAccessStorage),5,0);                //顺序存取存储器
    layout->addWidget(createCellWidget(tr("直接存取存储器"),DiagramItem::DirectAccessStorage),5,1);                    //直接存取存储器
    layout->addWidget(createCellWidget(tr("磁盘"),DiagramItem::Disk),6,0);                                              //磁盘
    layout->addWidget(createCellWidget(tr("卡片"),DiagramItem::Card),6,1);                                            //卡片
    layout->addWidget(createCellWidget(tr("人工输人"),DiagramItem::ManualInput),7,0);                              //人工输人
    layout->addWidget(createCellWidget(tr("穿孔带"),DiagramItem::PerforatedTape),7,1);                           //穿孔带
    layout->addWidget(createCellWidget(tr("显示"),DiagramItem::Display),8,0);                                        //显示
    layout->addWidget(createCellWidget(tr("准备"),DiagramItem::Preparation),8,1);                                    //准备
    layout->addWidget(createCellWidget(tr("人工操作"),DiagramItem:: ManualOperation),9,0);                           //人工操作
    layout->addWidget(createCellWidget(tr("并行方式"),DiagramItem::ParallelMode),9,1);                               //并行方式
    layout->addWidget(createCellWidget(tr("循环界限"),DiagramItem::Hexagon),10,0);                                   //循环界限

    QToolButton *textButton = new QToolButton;
    textButton->setCheckable(true);
    buttonGroup->addButton(textButton, InsertTextButton);
    textButton->setIcon(QIcon(QPixmap(":/images/textpointer.png")));
    textButton->setIconSize(QSize(70,70));
    textButton->setToolTip("文本");
    QGridLayout *textLayout = new QGridLayout;
    textLayout->addWidget(textButton, 0, 0, Qt::AlignHCenter);
    // textLayout->addWidget(new QLabel(tr("Text")), 1, 0, Qt::AlignCenter);
    QWidget *textWidget = new QWidget;
    textWidget->setLayout(textLayout);
    layout->addWidget(textWidget, 0, 0);


    QWidget *itemWidget = new QWidget;
    itemWidget->setLayout(layout);
    //! [22]
    toolBox = new QToolBox;
    toolBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored));
    toolBox->setMinimumWidth(itemWidget->sizeHint().width());
    toolBox->addItem(itemWidget, tr("图元集"));
}
//! [22]



void MainWindow::changebackground() {
    QWidget *currentWidget = tabwidget->currentWidget();
    if (currentWidget) {
        QGraphicsView *currentView = qobject_cast<QGraphicsView *>(currentWidget);
        if (currentView) {
            QString fileName = QFileDialog::getOpenFileName(this, tr("选择背景图片"), "", tr("图片文件 (*.png *.jpg *.bmp)"));
            if (!fileName.isEmpty()) {
                QPixmap backgroundPixmap(fileName);
                backgroundPixmap.setMask(backgroundPixmap.createHeuristicMask()); // 创建一个有效的遮罩
                QBrush brush(backgroundPixmap.toImage()); // 将QPixmap转换为QImage
                currentView->setBackgroundBrush(brush);
                brush.setColor(QColor(brush.color().red(), brush.color().green(), brush.color().blue(), 10));
                currentView->setBackgroundBrush(brush);
            }
        }
    }
}


////////////////////////////////////////////////////////////向下看
bool MainWindow::saveSceneAsImageOrSvg() {
    savePicPath = loadSavePicPath();
    QString filter = tr("PNG Files (*.png);;JPEG Files (*.jpg *.jpeg);;SVG Files (*.svg);;All Files (*)");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image or SVG"), savePicPath, filter, nullptr, QFileDialog::Options());

    if (!fileName.isEmpty()) {
        // 存储保存文件的路径
        savePicPath = QFileInfo(fileName).absolutePath();
        // 保存 saveFilePath 到文件
        saveSavePicPath(savePicPath);

        // 根据文件扩展名选择保存方式
        if (fileName.endsWith(".svg", Qt::CaseInsensitive)) {
            QSvgGenerator svgGen;
            svgGen.setFileName(fileName);
            svgGen.setSize(QSize(view->width(), view->height()));
            svgGen.setViewBox(QRect(0, 0, view->width(), view->height()));

            QPainter painter;
            painter.begin(&svgGen);
            scene->render(&painter);
            painter.end();

            if (!QFile::exists(fileName)) {
                QMessageBox::warning(this, tr("Error"), tr("Unable to save the SVG."));
                return false;
            } else {
                QMessageBox::information(this, tr("Success"), tr("SVG saved successfully."));
            }
        } else {
            QPixmap pixmap = view->grab();
            if (!pixmap.save(fileName)) {
                QMessageBox::warning(this, tr("Error"), tr("Unable to save the image."));
                return false;
            } else {
                QMessageBox::information(this, tr("Success"), tr("Image saved successfully."));
            }
        }
    }
    return true;
}
//////////////////////////////


//! [23]
void MainWindow::createActions()
{
    toFrontAction = new QAction(QIcon(":/images/bringtofront.png"),tr("前进一层"), this);
    toFrontAction->setShortcut(tr("Ctrl+F"));
    toFrontAction->setStatusTip(tr("将元素置于上层"));
    connect(toFrontAction, &QAction::triggered, this, &MainWindow::bringToFront);
    //! [23]

    sendBackAction = new QAction(QIcon(":/images/sendtoback.png"), tr("置后一层"), this);
    sendBackAction->setShortcut(tr("Ctrl+T"));
    sendBackAction->setStatusTip(tr("将元素置于下层"));
    connect(sendBackAction, &QAction::triggered, this, &MainWindow::sendToBack);

    deleteAction = new QAction(QIcon(":/images/delete.png"), tr("&删除"), this);
    deleteAction->setShortcut(tr("Delete"));
    deleteAction->setStatusTip(tr("删除画布上的图形元素"));
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteItem);

    exitAction = new QAction(QIcon(":/images/exit.png"), tr("退出"), this);
    exitAction->setShortcut(tr("Esc"));
    exitAction->setStatusTip(tr("退出"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);


    saveSceneAction = new QAction(QIcon(":/images/picture.png"), tr("&导出图像"), this);
    saveSceneAction->setShortcut(tr("Ctrl+E"));
    saveSceneAction->setStatusTip(tr("导出图像"));
    connect(saveSceneAction, &QAction::triggered, this, &MainWindow::saveSceneAsImageOrSvg);


    saveFileAction = new QAction(QIcon(":/images/outload.png"), tr("&保存工程"), this);
    saveFileAction->setShortcut(tr("Ctrl+S"));//这个是用来编辑快捷键位的
    saveFileAction->setStatusTip(tr("存储工程文件"));
    connect(saveFileAction, &QAction::triggered, this, &MainWindow::savefile);

    loadFileAction = new QAction(QIcon(":/images/inload.png"), tr("&读取工程"), this);
    loadFileAction->setShortcut(tr("Ctrl+O"));//这个是用来编辑快捷键位的
    loadFileAction->setStatusTip(tr("读取工程文件"));
    connect(loadFileAction, &QAction::triggered, this, &MainWindow::loadfile);

    boldAction = new QAction(tr("字体加粗"), this);
    boldAction->setCheckable(true);
    QPixmap pixmap(":/images/bold.png");
    boldAction->setIcon(QIcon(pixmap));
    boldAction->setShortcut(tr("Ctrl+B"));
    connect(boldAction, &QAction::triggered, this, &MainWindow::handleFontChange);

    italicAction = new QAction(QIcon(":/images/italic.png"), tr("倾斜字体"), this);
    italicAction->setCheckable(true);
    italicAction->setShortcut(tr("Ctrl+I"));
    connect(italicAction, &QAction::triggered, this, &MainWindow::handleFontChange);

    underlineAction = new QAction(QIcon(":/images/underline.png"), tr("下划线"), this);
    underlineAction->setCheckable(true);
    underlineAction->setShortcut(tr("Ctrl+U"));
    connect(underlineAction, &QAction::triggered, this, &MainWindow::handleFontChange);

    aboutAction = new QAction(QIcon(":/images/logo.png"), tr("关于我们"), this);
    aboutAction->setShortcut(tr("F1"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);


    newSceneAction = new QAction(QIcon(":/images/newfile.png"), tr("&创建新页面"), this);
    newSceneAction->setShortcut(tr("Ctrl+N"));
    connect(newSceneAction, &QAction::triggered, this, &MainWindow::newScene);

    fillAction = new QAction(QIcon(":/images/fillcolor.png"), tr("填充图形颜色"), this);
    connect(fillAction, &QAction::triggered, this, &MainWindow::fillButtonTriggered);

    lineAction = new QAction(QIcon(":/images/linecolor.png"), tr("填充线条颜色"), this);
    connect(lineAction, &QAction::triggered, this, &MainWindow::lineButtonTriggered);

    textAction = new QAction(QIcon(":/images/textpointer.png"), tr("文本颜色"), this);
    connect(textAction, &QAction::triggered, this, &MainWindow::textButtonTriggered);

    copyAction = new QAction(QIcon(":/images/copy.png"), tr("复制"), this);
    copyAction->setShortcut(tr("Ctrl+C"));
    copyAction->setStatusTip(tr("Copy item to clipboard"));
    connect(copyAction, &QAction::triggered, this, &MainWindow::copyItems);

    pasteAction = new QAction(QIcon(":/images/paste.png"), tr("粘贴"), this);
    pasteAction->setShortcut(tr("Ctrl+V"));
    pasteAction->setStatusTip(tr("Paste item from clipboard"));
    connect(pasteAction, &QAction::triggered, this, &MainWindow::pasteItemsFromMenu);

    cutAction = new QAction(QIcon(":/images/cut.png"), tr("剪切"), this);
    cutAction->setShortcut(tr("Ctrl+X"));
    cutAction->setStatusTip(tr("Cut item from clipboard"));
    connect(cutAction, &QAction::triggered, this, &MainWindow::cutItems);

    changeAction = new QAction(QIcon(":/images/back.png"), tr("背景图片设置"), this);
    changeAction->setShortcut(tr("Ctrl+Alt+B"));
    changeAction->setStatusTip(tr("背景图片设置"));
    connect(changeAction, &QAction::triggered, this, &MainWindow::changebackground);
    // undoAction = new QAction(tr("&Undo"), this);
    // undoAction->setShortcuts(QKeySequence::Undo);
    // undoAction->setStatusTip(tr("Undo the last operation"));
    // connect(undoAction, &QAction::triggered, undoStack, &QUndoStack::undo);

    // redoAction = new QAction(tr("&Redo"), this);
    // redoAction->setShortcuts(QKeySequence::Redo);
    // redoAction->setStatusTip(tr("Redo the last operation"));
    // connect(redoAction, &QAction::triggered, undoStack, &QUndoStack::redo);

    findAction = new QAction(QIcon(":/images/find.png"),tr("&查找"), this);
    findAction->setShortcut(tr("Ctrl+F"));
    connect(findAction, &QAction::triggered, this, &MainWindow::openFindReplaceDialog);

    combineAction = new QAction(QIcon(":/images/com.png"),tr("&组合"), this);
    connect(combineAction, &QAction::triggered, this, &MainWindow::combination);

    cancelcombineAction = new QAction(QIcon(":/images/cancelcom.png"),tr("&取消组合"), this);
    connect(cancelcombineAction, &QAction::triggered, this, &MainWindow::cancelCombination);



    undoAction = new QAction(QIcon(":/images/undo.png"),tr("&撤销"), this);
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setStatusTip(tr("Undo the last operation"));
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);

    redoAction = new QAction(QIcon(":/images/redo.png"),tr("&恢复"), this);
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setStatusTip(tr("Redo the last operation"));
    connect(redoAction, &QAction::triggered, this, &MainWindow::redo);


}

//! [24]
void MainWindow::createMenus()
{

    QMenu*exitMenu = menuBar()->addMenu(tr("&退出"));
    exitMenu->addAction(exitAction);

    // 在添加新菜单项之前，清空菜单
    fileMenu = menuBar()->addMenu(tr("&文件"));
    fileMenu->addSeparator();
    fileMenu->addAction(newSceneAction);
    fileMenu->addAction(saveFileAction);
    fileMenu->addAction(loadFileAction);
    fileMenu->addAction(saveSceneAction);



    itemMenu = menuBar()->addMenu(tr("&编辑"));
    itemMenu->addAction(changeAction);
    itemMenu->addAction(deleteAction);
    itemMenu->addSeparator();
    itemMenu->addAction(toFrontAction);
    itemMenu->addAction(sendBackAction);
    itemMenu->addAction(fillAction);
    itemMenu->addAction(copyAction);
    itemMenu->addAction(pasteAction);
    itemMenu->addAction(cutAction);
    itemMenu->addAction(undoAction);
    itemMenu->addAction(redoAction);


    aboutMenu = menuBar()->addMenu(tr("&帮助"));
    aboutMenu->addSeparator();
    aboutMenu->addAction(aboutAction);
}
//! [24]

//! [34]
void MainWindow::backgroundChanged(int index)
{
    QString imagePath = backgroundComboBox->itemData(index).toString();
    scene->setBackgroundBrush(QPixmap(imagePath));
    scene->update();
    view->update();
}
//! [34]
//! [25]
void MainWindow::createToolbars()
{
    //! [25]
    fileToolBar = addToolBar(tr("文件"));  //相当于 文件这个 toobar 里面加上 new save 和 delete 三种action 删掉了就没了
    fileToolBar->addAction(exitAction);
    fileToolBar->addAction(newSceneAction);
    fileToolBar->addAction(saveSceneAction);
    fileToolBar->addAction(deleteAction);


    fontCombo = new QFontComboBox();
    fontCombo->setCurrentFont(QFont("宋体", 10)); // 设置默认字体为Arial，字号为10
    connect(fontCombo, &QFontComboBox::currentFontChanged,this, &MainWindow::currentFontChanged);

    fontSizeCombo = new QComboBox;
    fontSizeCombo->setMinimumWidth(80); // 设置最小宽度为100像素
    fontSizeCombo->setEditable(true);
    fontSizeCombo->addItem("8");
    fontSizeCombo->addItem("10");
    fontSizeCombo->addItem("12");
    fontSizeCombo->addItem("14");
    fontSizeCombo->addItem("16");
    fontSizeCombo->addItem("18");
    fontSizeCombo->addItem("20");
    fontSizeCombo->setCurrentIndex(2); // 设置默认字号为12
    // for (int i = 8; i < 30; i = i + 2)
    //     fontSizeCombo->addItem(QString().setNum(i));
    QIntValidator *validator = new QIntValidator(2, 64, this);
    fontSizeCombo->setValidator(validator);
    connect(fontSizeCombo, &QComboBox::currentTextChanged,this, &MainWindow::fontSizeChanged);



    // 创建背景样式下拉框
    backgroundComboBox = new QComboBox;
    backgroundComboBox->setToolTip(tr("更改画布样式")); // 设置鼠标悬停提示
    backgroundComboBox->addItem(QIcon(":/images/background1.png"),tr("蓝白网格"), QVariant(":/images/background1.png"));
    backgroundComboBox->addItem(QIcon(":/images/background2.png"),tr("白色网格"), QVariant(":/images/background2.png"));
    backgroundComboBox->addItem(QIcon(":/images/background3.png"),tr("灰白网格"), QVariant(":/images/background3.png"));
    backgroundComboBox->addItem(QIcon(":/images/background4.png"),tr("无网格线"), QVariant(":/images/background4.png"));
    backgroundComboBox->setCurrentIndex(2); // 默认选中 "No Grid"
    backgroundComboBox->setCurrentText("无网格线");
    connect(backgroundComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &MainWindow::backgroundChanged);

    textToolBar = addToolBar(tr("Font"));
    textToolBar->addWidget(fontCombo);
    textToolBar->addWidget(fontSizeCombo);
    textToolBar->addAction(boldAction);
    textToolBar->addAction(italicAction);
    textToolBar->addAction(underlineAction);

    colorToolBar = addToolBar(tr("Color"));
    colorToolBar->addAction(textAction);
    colorToolBar->addAction(fillAction);

    colorToolBar->addWidget(backgroundComboBox);

    QToolButton *pointerButton = new QToolButton;
    pointerButton->setCheckable(true);
    pointerButton->setChecked(true);
    pointerButton->setToolTip("移动");
    pointerButton->setIcon(QIcon(":/images/move.png"));
    QToolButton *linePointerButton = new QToolButton;
    linePointerButton->setCheckable(true);
    linePointerButton->setToolTip("添加线条");
    linePointerButton->setIcon(QIcon(":/images/addline.png"));

    pointerTypeGroup = new QButtonGroup(this);
    pointerTypeGroup->addButton(pointerButton, int(DiagramScene::MoveItem));
    pointerTypeGroup->addButton(linePointerButton, int(DiagramScene::InsertLine));
    connect(pointerTypeGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::pointerGroupClicked);

    sceneScaleCombo = new QComboBox;
    QStringList scales;
    scales << tr("50%") << tr("75%") << tr("100%") << tr("125%") << tr("150%")<< tr("200%")<< tr("400%")<< tr("800%");
    //往上面添加字符串就可以直接实现
    sceneScaleCombo->addItems(scales);
    sceneScaleCombo->setCurrentIndex(2);
    connect(sceneScaleCombo, &QComboBox::currentTextChanged,this, &MainWindow::sceneScaleChanged);

    pointerToolbar = addToolBar(tr("线段类型"));
    pointerToolbar->addWidget(pointerButton);
    pointerToolbar->addWidget(linePointerButton);
    pointerToolbar->addWidget(sceneScaleCombo);

    findToolBar = addToolBar(tr("查找"));
    findToolBar->addAction(findAction);
    findToolBar->addAction(combineAction);
    findToolBar->addAction(cancelcombineAction);

    //! [27]
}
//! [27]

//! [28]
QWidget *MainWindow::createBackgroundCellWidget(const QString &text, const QString &image)
{
    QToolButton *button = new QToolButton;
    button->setText(text);
    button->setIcon(QIcon(image));
    button->setIconSize(QSize(50, 50));
    button->setCheckable(true);
    backgroundButtonGroup->addButton(button);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(button, 0, 0, Qt::AlignHCenter);
    layout->addWidget(new QLabel(text), 1, 0, Qt::AlignCenter);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    return widget;
}
//! [28]

//! [29]
//这个挺重要的 这个主要涉及到了除了文字之外的 左侧菜单栏所有图形
QWidget *MainWindow::createCellWidget(const QString &text, DiagramItem::DiagramType type)
{

    DiagramItem item(type, itemMenu);
    QIcon icon(item.image());

    QToolButton *button = new QToolButton;
    button->setIcon(icon);
    button->setIconSize(QSize(70, 70)); //强烈建议这个大小 和 文字大小保持一致
    button->setCheckable(true);
    button->setToolTip(text);
    buttonGroup->addButton(button, int(type));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(button, 0, 0, Qt::AlignHCenter);
    // layout->addWidget(new QLabel(text), 1, 0, Qt::AlignCenter);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    return widget;
}

//! [30]
template<typename PointerToMemberFunction>
//! [31]

//! [32]
QIcon MainWindow::createColorIcon(QColor color)
{
    QPixmap pixmap(20, 20);
    QPainter painter(&pixmap);
    painter.setPen(Qt::NoPen);
    painter.fillRect(QRect(0, 0, 20, 20), color);

    return QIcon(pixmap);
}
//! [32]
void MainWindow::undo() {
    if (!undoStack.isEmpty()) {
        QString stateFilePath = undoStack.pop();
        if(stateFilePath == "0.fcproj") return;
        loadfilestack(undoStack.top());
        redoStack.push(stateFilePath);
    }else{
        qDebug()<<"kongde";
    }
}

void MainWindow::redo() {
    if (!redoStack.isEmpty()) {
        QString stateFilePath = redoStack.pop();
        loadfilestack(stateFilePath);
        undoStack.push(stateFilePath);
    }
}

void MainWindow::autoCleanStack() {
    const int maxFiles = 20;  // 设置最大文件数量
    QDir dir("stacks");
    QStringList files = dir.entryList(QStringList() << "*.fcproj", QDir::Files);
    QStack<QString> temporarystack;
    if (files.size() >= maxFiles) {
        // 清空栈或删除文件
        while (files.size() > maxFiles-2) {  // 保留一定数量的文件
            QString filePath = dir.filePath(files.first());
            QFile::remove(filePath);
            files.removeFirst();
            fileCount--;  // 更新文件计数
        }
        while (!undoStack.isEmpty() && undoStack.size() > maxFiles - 2) {
            QString s = undoStack.pop();
            temporarystack.push(s);
        }

        undoStack.clear();
        while (!temporarystack.isEmpty()) {
            QString s = temporarystack.pop();
            undoStack.push(s);
        }
    }
}

//保存文件
QString MainWindow::savefilestack(){

    QString currentDir = QDir::currentPath();
    qDebug()<<currentDir;
    // 创建 undo 文件夹路径
    QString defaultPath = QDir(currentDir).filePath("stacks");
    QString textFile = QString::number(path) +".fcproj";
    QDir dir(defaultPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString filePath = dir.filePath(textFile);
    // 保存 saveFilePath 到文件
    // saveSaveFilePath(textFile);

    // 执行保存操作
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("保存失败"), tr("无法打开或写入工程信息."));
        return "default";
    }
    fileCount++;  // 增加文件计数
    autoCleanStack();  // 调用清理函数
    // 创建 QTextStream 对象，并指定编码为 UTF-8
    QTextStream out(&file);
    // 获取 DiagramItem 结构体指针列表
    QList<WriteDiagramItem*> diagramItems = getStructList();
    // 写入 DiagramItem 结构体列表的大小
    out << "DT_Size_" << diagramItems.size() << "\n";
    // 遍历 DiagramItem 结构体指针列表
    for (WriteDiagramItem* itemPtr : diagramItems) {
        // 替换 internalText 和 texttype 中的空格为星号
        QString modifiedInternalText = itemPtr->internalText.replace(" ", "*");
        QString modifiedTextType = itemPtr->texttype.replace(" ", "*");

        // 写入结构体成员
        out << itemPtr->x << " "
            << itemPtr->y << " "
            << itemPtr->width << " "
            << itemPtr->height << " "
            << itemPtr->type << " ";
        for (int i = 0; i < 4; ++i) {
            out << itemPtr->rbg[i] << " ";
        }
        out << modifiedInternalText << " " // 使用替换后的 internalText
            << itemPtr->itemtype << " "
            << modifiedTextType << " " // 使用替换后的 texttype
            << itemPtr->textsize << " "
            << itemPtr->boldtype << " "
            << itemPtr->itlatic << " ";
        for (int i = 0; i < 4; ++i) {
            out << itemPtr->textrbg[i] << " ";
        }
        out << "\n";
    }
    // 关闭文件
    file.close();

    // 获取 DiagramPath 结构体指针列表
    // 获取 DiagramPath 结构体指针列表
    QList<struct WriteDiagramPath *> diagramPathsPtrs = getStructList1();
    // 如果用户取消了文件选择或列表为空，则不执行任何操作
    if(!diagramPathsPtrs.isEmpty()){
        if (textFile.isEmpty() || diagramPathsPtrs.isEmpty()) {
            qDebug()<<"empty";
            saveFileSuccess = false;
            return "default";
        }
        // 以追加模式打开文件，以写入 DiagramPath 数据
        if (!file.open(QIODevice::Append)) {
            QMessageBox::critical(this, tr("保存失败"), tr("无法打开或写入工程信息."));
            saveFileSuccess = false;
            return "default";
        }
        // 写入 DiagramPath 结构体列表的大小
        out << "LN_Size_" << diagramPathsPtrs.size() << "\n";

        // 遍历 DiagramPath 结构体指针列表
        for (struct WriteDiagramPath *pathPtr : diagramPathsPtrs) {
            if (pathPtr) {
                // 写入结构体成员
                out << pathPtr->start << " "
                    << pathPtr->startp << " "
                    << pathPtr->end << " "
                    << pathPtr->endp << "\n";
            }
        }
    }
    // 关闭文件
    file.close();
    undoStack.push(textFile);
    path++;
    return textFile;
}

void MainWindow::loadfilestack(QString str) {
    // QString currentDir = QDir::currentPath();
    qDebug()<<"执行loadfilesstack";
    QDir dir("stacks");

    QString filePath = dir.filePath(str);

    // 执行保存操作
    QFile file(filePath);

    // 以只读模式打开文件
    if (file.open(QIODevice::ReadOnly)) {
        // 创建 QTextStream 对象
        QTextStream in(&file);

        // 创建新的列表用于存放读取的数据
        QList<ReadDiagramItem> readDiagramItems;

        // 读取 DiagramItem 结构体列表的大小
        QString sizeString;
        in >> sizeString;
        if (sizeString.startsWith("DT_Size_")) {
            quint32 diagramItemCount = sizeString.mid(8).toUInt(); // 去掉前缀并转换为整数
            for (quint32 i = 0; i < diagramItemCount; ++i) {
                ReadDiagramItem item;
                in >> item.x >> item.y >> item.width >> item.height >> item.type;
                for (int j = 0; j < 4; ++j) {
                    in >> item.rbg[j];
                }
                // 读取 internalText 并替换星号为空格
                QString internalText;
                in >> internalText;
                item.internalText = internalText.replace("*", " ");
                in >> item.itemtype;
                // 读取 texttype 并替换星号为空格
                QString textType;
                in >> textType;
                item.texttype = textType.replace("*", " ");
                in >> item.textsize;
                // 读取布尔值需要特殊处理
                QString boldTypeStr;
                in >> boldTypeStr;
                item.boldtype = (boldTypeStr == "true");

                QString itlaticStr;
                in >> itlaticStr;
                item.itlatic = (itlaticStr == "true");

                for (int j = 0; j < 4; ++j) {
                    in >> item.textrbg[j];
                }

                readDiagramItems.append(item);
            }

        }

        QList<ReadDiagramPath> readDiagramPaths;
        in >> sizeString;
        if (sizeString.startsWith("LN_Size_")) {
            quint32 diagramPathCount = sizeString.mid(8).toUInt(); // 去掉前缀并转换为整数
            for (quint32 i = 0; i < diagramPathCount; ++i) {
                ReadDiagramPath path;
                in >> path.start >> path.startp >> path.end >> path.endp;
                // 调试输出每个读取到的 DiagramPath
                qDebug() << "Read DiagramPath:";
                qDebug() << "start: " << path.start;
                qDebug() << "startp: " << path.startp;
                qDebug() << "end: " << path.end;
                qDebug() << "endp: " << path.endp;
                readDiagramPaths.append(path);
            }
        }
        scene->clear();

        QList<DiagramItem *> DiagramItemList;

        foreach (ReadDiagramItem item,readDiagramItems) {
            DiagramItem *item1=new DiagramItem(static_cast<DiagramItem::DiagramType>(item.itemtype),itemMenu);
            item1->setPos(QPoint(item.x,item.y));
            item1->setFixedSize(QSize(item.width,item.height));
            QColor itemcolor(item.rbg[0],item.rbg[2],item.rbg[1],item.rbg[3]);
            QColor textcolor(item.textrbg[0],item.textrbg[2],item.textrbg[1],item.textrbg[3]);
            item1->m_color=itemcolor;
            item1->textItem->setPlainText(item.internalText);
            item1->textItem->font().setPointSize(item.textsize);
            item1->textItem->font().setFamily(item.texttype);
            item1->textItem->font().setBold(item.boldtype);
            item1->textItem->font().setItalic(item.itlatic);
            item1->textItem->setDefaultTextColor(textcolor);
            scene->addItem(item1);
            DiagramItemList.append(item1);
        }
        int size = DiagramItemList.size();
        qDebug()<<"size: "<< size;
        foreach (ReadDiagramPath item,readDiagramPaths) {
            DiagramItem *startItem = DiagramItemList.at(item.start-1 );
            DiagramItem *endItem = DiagramItemList.at(item.end-1 );
            qDebug()<<scene->items();
            DiagramItem::TransformState startState = static_cast<DiagramItem::TransformState>(item.startp);
            DiagramItem::TransformState endState = static_cast<DiagramItem::TransformState>(item.endp);

            DiagramPath *item1=new DiagramPath(startItem,endItem,startState,endState);

            startItem->addPathes(item1);
            startItem->marks[item1] = "1" + QString::number(startState);
            qDebug()<<startItem->marks;
            endItem->addPathes(item1);
            endItem->marks[item1] = "0" + QString::number(endState);
            qDebug()<<endItem->marks;
            qDebug()<<startState<<endState;
            item1->updatePath();
            item1->setZValue(-1000.0);

            scene->addItem(item1);

        }        // 提示用户读取成功
        // QMessageBox::information(this, tr("加载完成"), tr("成功加载工程."));
    } else {
        // 文件打开失败，提示用户错误信息
        QMessageBox::critical(this, tr("加载失败"), tr("无法打开或读取文件信息."));
    }
}
