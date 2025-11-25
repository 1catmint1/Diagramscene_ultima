//程序运行开始的地方 -- 运行mainwindow
#include "mainwindow.h"
#include <QApplication>

int main(int argv, char *args[])
{
    QApplication app(argv, args);
    MainWindow mainWindow;
    // mainWindow.setGeometry(0, 0, 1920,1080);
    // mainWindow.show();
    mainWindow.showMaximized();     //其实直接使用showMaximized()就会实现自动铺满
    return app.exec();
}
