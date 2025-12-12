QT += core testlib widgets svg
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = test_diagramitems

# 测试源（把需要的实现源码也加入，以便链接）
SOURCES += test_diagramitems.cpp \
    ../mainwindow.cpp \
    ../deletecommand.cpp \
    ../diagramitem.cpp \
    ../diagramitemgroup.cpp \
    ../diagrampath.cpp \
    ../findreplacedialog.cpp \
    ../arrow.cpp \
    ../diagramtextitem.cpp \
    ../diagramscene.cpp \
    test_arrow_straight_connection.cpp \
    test_connectionline_style.cpp \
    test_diagramitem_properties.cpp \
    test_diagramitem_transform.cpp \
    test_diagrampath_connection.cpp \
    test_diagramtextitem_edit.cpp \
    test_findreplacedialog.cpp

# 把主工程的头文件列入 HEADERS 以触发 automoc 为带 Q_OBJECT 的类生成 moc 文件
HEADERS += ../mainwindow.h \
    ../deletecommand.h \
    ../diagramitem.h \
    ../diagramitemgroup.h \
    ../diagrampath.h \
    ../diagramscene.h \
    ../arrow.h \
    ../diagramtextitem.h \
    ../findreplacedialog.h

# 资源（如果实现里使用了 qrc）
RESOURCES += ../diagramscene.qrc

# 头文件搜索路径（tests 在工程根的 tests/）
INCLUDEPATH += ..

# 如有需要，补充其它 include 路径
# INCLUDEPATH += ../src
