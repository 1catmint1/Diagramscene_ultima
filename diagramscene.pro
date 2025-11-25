QT += widgets
QT += svg

requires(qtConfig(fontcombobox))

HEADERS	    =   mainwindow.h \
		deletecommand.h \
		diagramitem.h \
		diagramitemgroup.h \
		diagrampath.h \
		diagramscene.h \
		arrow.h \
		diagramtextitem.h \
		findreplacedialog.h
SOURCES	    =   mainwindow.cpp \
		deletecommand.cpp \
		diagramitem.cpp \
		diagramitemgroup.cpp \
		diagrampath.cpp \
		findreplacedialog.cpp \
		main.cpp \
		arrow.cpp \
		diagramtextitem.cpp \
		diagramscene.cpp
RESOURCES   =	diagramscene.qrc


# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/graphicsview/diagramscene
INSTALLS += target

FORMS +=
