QT += core testlib widgets svg
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = test_diagramitems

SOURCES += \
    test_main.cpp \
    test_scene_management.cpp \
    test_file_io.cpp \
    test_undo_redo.cpp \
    ../mainwindow.cpp \
    ../deletecommand.cpp \
    ../diagramitem.cpp \
    ../diagramitemgroup.cpp \
    ../diagrampath.cpp \
    ../findreplacedialog.cpp \
    ../arrow.cpp \
    ../diagramtextitem.cpp \
    ../diagramscene.cpp

HEADERS += \
    ../mainwindow.h \
    ../deletecommand.h \
    ../diagramitem.h \
    ../diagramitemgroup.h \
    ../diagrampath.h \
    ../diagramscene.h \
    ../arrow.h \
    ../diagramtextitem.h \
    ../findreplacedialog.h

RESOURCES += ../diagramscene.qrc
INCLUDEPATH += ..
