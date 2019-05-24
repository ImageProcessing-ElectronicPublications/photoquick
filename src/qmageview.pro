######################################################################
# Automatically generated by qmake (2.01a) Sun Oct 29 20:36:57 2017
######################################################################

TEMPLATE = app
TARGET = qmageview
DEPENDPATH += .
INCLUDEPATH += .
LIBS += -lgomp
QMAKE_CXXFLAGS = -fopenmp

# build dir
MOC_DIR = build
RCC_DIR = build
UI_DIR = build
OBJECTS_DIR = build
mytarget.commands += $${QMAKE_MKDIR} build

# Input
HEADERS += exif.h image.h main.h photogrid.h dialogs.h
SOURCES += exif.cpp image.cpp main.cpp photogrid.cpp dialogs.cpp filters.cpp
RESOURCES += resources.qrc
FORMS += mainwindow.ui resize_dialog.ui photogrid_dialog.ui gridsetup_dialog.ui

# install
INSTALLS += target
target.path = /usr/local/bin

