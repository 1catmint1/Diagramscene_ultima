QT += core testlib widgets svg
QT += core widgets svg  # 保留其他模块
CONFIG += console
CONFIG -= app_bundle
include(./googletest-master/gtest_dependency.pri)
TEMPLATE = app
TARGET = test_diagramitems

# ========== 新增：覆盖率检测核心配置（Debug模式生效） ==========
# 1. 强制Debug模式（Release会优化代码，覆盖率统计失真）
CONFIG += debug
# 2. 添加gcov覆盖率编译/链接选项
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
# 3. 链接gcov库（Windows MinGW/Linux 必需）
LIBS += -lgcov
# ========== 覆盖率配置结束 ==========
# 覆盖率编译选项
contains(QMAKE_CXX, g++) {
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage -O0 -g
    QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
    LIBS += -lgcov
}
SOURCES += \
    test_arrow_connection_system.cpp \
    test_arrow_straight_connection.cpp \
    test_component_signal_integrate.cpp \
    test_connectionline_style.cpp \
    test_dataflow_integrity.cpp \
    test_diagramitem_properties.cpp \
    test_diagramitem_transform.cpp \
    test_diagramitem_types_full.cpp \
    test_diagramitems_create.cpp \
    test_diagrampath_connection.cpp \
    test_diagrampath_path_dev.cpp \
    test_diagramtextitem_edit.cpp \
    test_findreplacedialog.cpp \
    test_main.cpp \
    test_performance_stress.cpp \
    test_performance_workflow.cpp \
    test_scene_management.cpp \
    test_file_io.cpp \
    test_shortcuts.cpp \
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

# ========== 关键：GTest + GMock 配置 ==========
# 头文件路径（GTest + GMock）
INCLUDEPATH += C:\Users\lenovo\Desktop\googletest-main\googletest\include \
               C:\Users\lenovo\Desktop\googletest-main\googlemock\include

# 库文件路径（编译好的 GTest + GMock）
LIBS += C:\Users\lenovo\Desktop\googletest-main\build\lib\libgtest.a \
        C:\Users\lenovo\Desktop\googletest-main\build\lib\libgtest_main.a \
        C:\Users\lenovo\Desktop\googletest-main\build\lib\libgmock.a \
        C:\Users\lenovo\Desktop\googletest-main\build\lib\libgmock_main.a

# 启用 C++11（GMock 必须）
CONFIG += c++11
# ========== GTest/GMock 配置结束 ==========
